# Lox language interpreter

An interpreter implementation for the lox language, written with the help of the wonderful book by Robert Nystrom - [Crafting Interpreters](https://craftinginterpreters.com/).

The source code itself is carefully separated into three components:
- `src/lox-common` - Various helpers and primitives of the lox language;
- `src/frontend` - The frontend pieces of the interpreter (Scanner, Importer, Parser, Resolver);
- `src/tree-walker` - The backend in the form of a tree-walk interpreter that supports expressions, statements, control flow, and functions with closures.

This separation should enable you to simply extend the implementation with another backend of your choosing. All you'd need to do is define your own AST visitor (interpreter/codegen), your own error class (optional), and some kind of a `Runner` class or a `run` function that puts the frontend and backend together.

The language itself is relatively simple as it was created purely for the book, and is missing some, in my opinion, *vital* features, such as IO and modularization. This leaves plenty of opportunity for playing around with various extensions.

# Notable features and extensions

## Modularization support via `import` statements

One feature that I felt was extremely important - the ability to reuse existing code from other files, is unfortunately missing from the language in the original design. You either put all the code in a single .lox file, or... uhh...

This implementation extends the language with the `import` statement:
```
import "path/to/code.lox";
```
where the only accepted argument is a string literal designating a path to the imported file.

The import resolution pass happens after scanning the tokens of the initial source, so it's slightly more robust than the `#include` macro: the files that were already `import`ed are not reimported again, non-global and conditional imports are forbidden. Otherwise, the behaviour is similar to using `#include` with properly guarded header files. Given that lox has no declarations or the ability to form pointers/references, there's no need to break the source code into 'translation units', and so this import mechanism happens to be fully sufficient.


## Closure support for stack-based functions

Since execution of the lox's `Function` happens inside the backend-dependant `Function::operator()` internally, the lox's function frame in this implementation is also allocated on the stack. But, because the lifetime of the functions in lox can extend beyond the scope of declaration (functions are first-class), references to non-local variables as part of closure can access the values outside their lifetime, due to the corresponding stack frame being gone
(see the [Funarg problem](https://en.wikipedia.org/wiki/Funarg_problem)).

Because this implementation is done in C++, I did not have the benefit of the GC to handle the lifetime extension of captured variables. At the same time, since the `Value` type was aready implemented as a variant, I wanted to preserve the value-semantics for `Value`s and did not want to introduce a *heap promotion* mechanism, where captured variables would be moved from the stack to the heap.

Instead, the captured variables are *copied* directly into the closure of a function at the point of declaration and exist there as a separate, not connected to the enclosing environment, state. This is a semantic departure from the design proposed in a book, as you can no longer indirectly mutate non-local state:

```lox
var message = "hello";

fun f() {
    message = "bye"; // Modifies a copy, not the original
}

f();

print message; // Prints "hello"
```

But these semantics still leave enough room for useful patters:

```lox
fun make_iter() {
    var i = 0;
    fun inner() {
        var temp = i;
        i = i + 1;
        return temp;
    }
    return inner;
}

fun make_rand(a, b) {
    fun inner() {
        // Builtin rand() returns random double from 0.0 to 1.0
        return (b - a) * rand() + a;
    }
    return inner;
}
```

Also see the [docs/CLOSURES.md](docs/CLOSURES.md) for detailed notes on the reasoning behind this particular design, and a small tale about my struggles to implement various features like static name resolution and recursion alongside closures.

## Robust error reporting

 TODO

# Building

Requirements:
CMake, vcpkg.

Dependencies (installed by vckpg):
boost, fmt, cxxopts, doctest.

From project root:
```bash
mkdir build

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

cmake --build build
```
