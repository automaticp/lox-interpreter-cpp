# Motivation

The exact semantics of closures have been bothering me for more than half a week. Implementing them without the help of some kind of GC turned out to be a major pain point. After juggling tonns of both pedantic and implementation details, I realized that I only overload my brain this way, and don't progress towards any kind of result. I need a place to vent.

I thereby will use this document to develop and outline (?) the closure semantics that are:
1. Relatively easy to implement in a language with stack-based functions;
2. Relatively easy to implement in a language without GC;
3. Still somewhat sensible from the user point of view.

(prioritized exactly in that order)

# Terminology

(Could be incomplete, or wrong)

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


The second point about the capture being read-only might seem meaningless. And partly it is. Why restrict modification of the of the value if it has already been copied? No mutation can affect the non-local state, if the variable has been copied with all of its guts.

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

TODO

declaration time obv.
