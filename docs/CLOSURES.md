# Motivation

The exact semantics of closures have been bothering me for more than half a week. Implementing them without the help of some kind of GC turned out to be a major pain point. After juggling tonns of both pedantic and implementation details, I realized that I only overload my brain this way, and don't progress towards any kind of result. I needed a place to vent.

I thereby will use this document to develop and outline the closure semantics that are:
1. Relatively easy to implement in a language with stack-based functions;
2. Relatively easy to implement in a language without GC;
3. Still somewhat sensible from the user point of view.

(prioritized exactly in that order)

# Terminology

(could be incomplete, sorry)

**Environment** - essentially, a map: name -> value. Depending on it's lifetime, can be either *stack-bound* - created and destroyed with the local function scope or block scope, or *value-bound* - created with the instance of the `Value` variant such as `Function` with closure or `Object`.

**Capture/Capture variables** - a set of variables, bound at the specific *capture time* to the *closure environment*.

**Closure/Closure environment** - a value-bound enviroment separate from the local stack-bound environment of the function body, that countains all the referenced in the local scope, but not declared, variables.

**Capture time** - a specific moment when referenced in the function body names are bound to the *closure environment*.


Implementation-wise you can imagine that every `Function` is a class with the closure environment as it's member:

```c++
class Function {
private:
    Environment closure_{ nullptr };
    const FunStmt* declaration_;

public:
    Function(const FuncStmt* decl) :
        declaration_{ decl } {}

    // ...
};
```
Where the `Environment` itself is a map and a pointer to the enclosing environment:
```c++
class Environment {
private:
    Environment* enclosing_;
    map_t<std::string, Value> map_;
public:
    Environment(Environment* enclosing) :
        enclosing_{ enclosing } {}

    // ...
};
```

As you can see, the closure itself has no enclosing environments.
The function must consume all the neccessary defenitions from the enclosing scopes at *capture time*, and 'flatten' them into a it's closure.


# Specification

## Captured variables

Variables in the closure environment are:

1. Captured by COPY
2. READ-ONLY


The first point comes directly from the implementation of the `Value` and the fact that instances of `Value` already adhere to value semantics, which makes their lifetime management straightforward.

```
fun outer() {
    var i = 0;
    fun inner() {
        // Copy of 'i' exists in the
        // closure of this scope
        return i;
    }
}
```

One of the variants of `Value` - `ValueHandle` can provide reference semantics in such cases, but it models C++'s non-owning pointers, and therefore gives no guarantees on the lifetime, making this simple program exhibit UB:

```
fun outer() {
    var i = 0;
    fun inner() {
        // ValueHandle of 'i' exists
        // in the closure of this scope
        return i;
    }
}

var f = outer();

// Dereference 'i' from the scope of 'outer()',
// whose stack-frame no longer exists:
print f(); // BOOM!
```

This is where the argument about GCs or reference counting comes in, where in the most simple case `i` would be of type `shared_ptr<Value>`. This, however, would be too much of a redesign hassle, and, in a way, would nullify my, possibly futile, efforts to preserve variant-ness of `Value`.


The second point about the capture being read-only might seem meaningless. And partly it is. Why restrict modification of the value if it has already been copied? No mutation can affect the non-local state, if the variable has been copied with all of its guts.

But assume you, as a user, are unaware about the strict copy semantics for closures. You might want to do funny things like:


```
var phrase; // default init to nil

fun prepare_greeting() {
    phrase = "Hello!";
}

fun change_my_mind() {
    phrase = "Bye!";
}

fun greet() {
    print phrase;
}

prepare_greeting();
change_my_mind();

greet();
```

Yet neither of the two first functions will have any effect on the `phrase`. In the end, the program will print:
```
nil
```

It's the user's expectation of the reference semantics is where the confusion can arise. Here's where we gut their dreams: even if it was a reference, it's a reference to a value you cannot change.

*\~\~and this bird you cannot chaaaaange\~\~*


## Capture time

Another detail that impacts the user's reasoning about closures is the time when the capture is created. Take a look at this perfeclty legal python code:

```python
def outer():
    def inner():
        return x
    x = 5
    return inner

f = outer()
print(f())
```

Or, better yet, this one:

```python
def outer():
    def inner():
        return x
    return inner

x = 5

f = outer()
print(f())
```

Ohhh, lemme pull out a funny one:

```python
def outer():
    def inner():
        return x
    return inner

def do():
    x = 6
    f = outer()
    print(f())

do()
```

The last one isn't actually legal, but the first two are ablosutely fine pieces of python code.

So, can you tell exactly WHEN the `x` is captured in the closure of `inner()`?

Python is GCed and employs reference semantics, so it seems that it's answer is: ALWAYS.
That is, as long as the referenced name is defined in the enclosing scope before it is used in the body of a function with closure, all is fine and dandy.

Here's a riddle: could you emulate this behavior with copy semantics for closures? Copy semantics mean for us that the answer 'always' is not feasible, we cannot always 'track' a variable. We could do some kind of late binding, and copy only when the variable is referenced:

```
var min = 0;
var max = 100;

fun make_rand() {
    fun inner() {
        // Variables will only be bound when evaluated
        return (max - min) * rand() + min;
    }
    return inner;
}

var my_rand = make_rand(); // No binding yet


print my_rand(); // Bind NOW

// Prints random number from 0 to 100

min = 42; // Oops

print my_rand(); // Bind NOW

// Prints random number from 42 to 100

```

The apparent behavior here is the same as the one proposed by Robert Nystrom in his book.

Here's a slightly more realistic example:

```
fun make_rand(min, max) {
    fun inner() {
        // Variables will only be bound when evaluated
        return (max - min) * rand() + min;
    }
    return inner;
}

var my_rand = make_rand(0, 100); // No binding yet

print my_rand(); // Bind now??? To what?
```

At the point of binding, the local variables `min` and `max` are already GONE, together with the scope of `make_rand()`. Late binding, more like, *too late* binding, amright?

*I've decided to start writing this document after I discovered that this problem has an entire [wikipedia page](https://en.wikipedia.org/wiki/Funarg_problem) dedicated to it. At the very least, it validates my feelings: I am not alone.*

To be fair, there are not that many options left, it's clear that we need to bind (read: copy), while we are still in the scope that contains the `fun` declaration. Still, we could either bind at the declaration point, or when the function leaves it's declaration scope:

```
fun make_f() {
    var i = 0;

    fun f() { // <-- Bind here
        return i;
    }

    i = 1;
    return f; // <-- Or here
}

var my_f = make_f();

print my_f();
```
Depending on this choice made, the above program will print either `0` or `1`. Here, the function `f` leaves the scope *upwards* (as in, up the function stack), it could also leave *downwards*:

```
fun consume_f(f) {
    print f();
}

fun print_result_of_f() {
    var i = 0;

    fun f() {
        return i;
    }

    i = 1;
    consume_f(f); // Leave downwards
}

print_result_of_f();
```

The nuances of implementing the second option, where the binding happens when the function leaves the scope, do not really make me favour it. I'd have to do implement the logic in the following way:
- If we are copying the function, check what scope we're copying from:
  - If it is the same as the declaration scope, then bind all referenced variables from the enclosing scopes;
  - If not, do nothing, copy directly.

This scope checking might require collecting more meta information during static analysis stage. But it's not the collecting part, it's thinking about how to put all that garbage to good use is what scares me.

The benefit of this approach is that closure is not created when the function does not leave the scope of declaration, saving a bit of memory and performance.


The alternative is a bit easier to implement, we just take a list of all referenced names, resolve them and copy the values one-by-one at the point of declaration of the function itself. I haven't yet thought about all possible cases where this might cause issues, but I'll go through with it for now, and we'll see what comes out of it later.

**Capture at the declaration time** it is. YOLO, anyone?


# Static resolution with closures


The static analysis stage of this implementation - `Resolver` has 2 jobs:
- Recurse through the entire AST and statically check the validity of the used identifiers (define before use, no initialization from self, etc.);
- For each referenced name, set the distance (depth) at which it's defenition can be found by recursing backwards through the environments from the environment enclosing current expression.

## Depth resolution

Current implementation of the `Resolver` produces a *depth map*: `const Expr*` -> `size_t`, where the key points to the identifier expression, and the value indicates how many times we have to recurse back to previous environments before arriving at the point of the defenition.

This comes in handy when we want to preserve lexical scoping and mimic mutability of identifiers through scopes:

```
var i = 0;
{
    i = 1;
    {
        print i;
    }
}
```

Behind this simple three-liner lies a bunch of gnarly implementation details:

```
var i = 0;       // 'i' is defined in env1.

{                // Create env2, with env1 as enclosing.

    i = 1;       // 'i' is referenced here in assignment:
                 // we recurse once to the enclosing env1 in order
                 // to retrieve the ValueHandle to 'i'
                 // and reassign the new value at the key of
                 // 'i' in the env1.

    {            // Create env3, with env2 as enclosing.

        print i; // 'i' is referenced here for evaluation:
                 // we recurse twice to env1 in order
                 // to retrieve the ValueHandle to 'i'
                 // and decay it to the undelying value.
    }
}
```
The resulting depth map is, however, deceivingly simple:
```
map_t<const Expr*, size_t>{
    { &expr1, 1 },           // i = 1;
    { &expr2, 2 }            // print i;
}
```

If now in our Interpreter we're going to use this map to directly index into the right enclosing scope:

*(the following c++ code snippets are simplified for the sake of the example)*

```c++
Value InterpretVisitor::operator(const VariableExpr& expr) {
    // ...

    map_t& depths = resolver.depth_map();

    auto it = depths.find(&expr);

    if (it != depths.end()) {
        ValueHandle handle =
            // Index directly into the resolved depth (it->second)
            environment.get_at(it->second, expr.identifier.lexeme);

        return handle;
    }
    // ...
    // Handle failure otherwise
}
```

Then we're bound to have trouble when instead of simple block scopes, we're going to face functions with closures.

```
var i = 0;
{
    fun f() {
        {
            return i; // The lexical depth is 3
        }
    }
}
```

The resolved (lexical) depth of `i` in this return statement is 3, but because functions with closures in this implementation 'flatten' all the enclosing environments into the capture environment, the maximum number of times it is possible to recurse from that return statement is just 2.

Ideally, we want to rely on the distance in scopes from the last function declaration, and use *that plus one* as the assigned depth for identifier not declared locally. The downside is that if during interpretation you'll need the original depth, there won't be a way to reproduce it.

Current implementation just keeps track of both the *lexical distance* and a *closure distance* - distance to the closure, if it were to be created later. Then we just take the minimum of the two:

```c++
size_t lexical_distance{ distance_to_var_decl(name) };

if (lexical_distance < num_of_scopes) {

    size_t closure_distance{ distance_to_enclosing_fun_scope() + 1 };

    resolver.set_depth(expr,
        std::min(lexical_distance, closure_distance));

} else {
    // Report undefined variable
}
```

## Immutability of capture variables

Another concern for me is that I've vouched to forbid modification of the captured variables. At the point of interpretation, depth map alone isn't enough to differentiate between non-locals and captured variables.

Fortunately, this can be resolved (yes) right in the static analysis stage: we see if the assigned-to name is beyond current function's scope, if it is, we just yell at the user:

```c++
size_t lexical_distance{ distance_to_var_decl(name) };

if (lexical_distance < num_of_scopes) {

    size_t local_depth_limit{ distance_to_enclosing_fun_scope() };

    if (lexical_distance > local_depth_limit) {
        // Report assignment to non-local variable
        // ...
    } else {
        resolver.set_depth(expr, lexical_distance);
    }
} else {
    // Report undefined variable
}
```



## Capture name list

TODO



*I'm all in for the Resolver storing much more information about the AST, but I, so far, haven't been able to concieve what that information should really be.*




# Recursion and closures

Ohhhh, the *fun* part...

TODO
