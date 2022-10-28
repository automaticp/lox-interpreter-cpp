# Motivation

The exact semantics of closures have been bothering me for more than half a week. Implementing them without the help of some kind of GC turned out to be a major pain point. After juggling tonns of both pedantic and implementation details, I realized that I only overload my brain this way, and don't progress towards any kind of result. I need a place to vent.

I thereby will use this document to develop and outline (?) the closure semantics that are:
1. Relatively easy to implement in a language with stack-based functions;
2. Relatively easy to implement in a language without GC;
3. Still somewhat sensible from the user point of view.

(prioritized exactly in that order)

# Terminology

(Could be incomplete, or wrong, sorry)

**Environment** - essentially, a map: name -> value. Depending on it's lifetime, can be either *stack-bound* - created and destroyed with the local function scope or block scope, or *value-bound* - created with the instance of the `Value` variant such as `Function` with closure or `Object`.

**Capture/Capture variables** - a set of variables, bound at the specific *capture time* to the *closure environment*.

**Closure environment** - a value-bound enviroment separate from the local stack-bound environment of the function body, that countains all the referenced in the local scope, but not declared, variables.

**Capture time** - a specific moment when referenced in the function body names are bound to the *closure environment*.

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

This scope checking might require collecting more meta information during static analysis stage. And I just donna wanna...

The benefit of this approach is that closure is not created when the function does not leave the scope of declaration, saving a bit of memory and performance.


The alternative is a bit easier to implement, we just take a list of all referenced names, resolve them and copy the values one-by-one at the point of declaration of the function itself.

I haven't yet thought about all possible cases where this might cause issues, but I'll go through with it for now, and we'll see what comes out of it later.

**Capture at the declaration time** it is. YOLO, anyone?


# Static resolution with closures


# Recursion and closures
