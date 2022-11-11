cmake_minimum_required(VERSION 3.20)
project(lox-interpreter)

option(LOX_BUILD_WITH_ASAN "Enable AddressSanitizer when building lox libraries" FALSE)
option(LOX_BUILD_WITH_UBSAN "Enable UBSanitizer when building lox libraries" FALSE)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
