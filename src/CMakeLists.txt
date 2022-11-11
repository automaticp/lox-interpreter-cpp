add_library(lox-config INTERFACE)

if (LOX_BUILD_WITH_ASAN)
    # Ehh, but whatever
    if (NOT MSVC)
        target_compile_options(lox-config INTERFACE "-fsanitize=address")
        target_link_options(lox-config INTERFACE "-fsanitize=address")
    else()
        message(WARNING "Building lox with ASan is only supported for GCC/Clang")
    endif()
endif()

if (LOX_BUILD_WITH_UBSAN)
    if (NOT MSVC)
        target_compile_options(lox-config INTERFACE "-fsanitize=undefined")
        target_link_options(lox-config INTERFACE "-fsanitize=undefined")
    else()
        message(WARNING "Building lox with UBSan is only supported for GCC/Clang")
    endif()
endif()



# Common utilities and primitives of lox
file(GLOB LOX_COMMON_SOURCES CONFIGURE_DEPENDS lox-common/*.cpp)
add_library(lox-common STATIC "${LOX_COMMON_SOURCES}")
target_compile_features(lox-common PUBLIC cxx_std_20)
target_include_directories(lox-common PUBLIC lox-common/)

target_link_libraries(lox-common
    PUBLIC
        fmt::fmt
        Boost::boost
        Boost::container
    PRIVATE
        lox-config
)
# Link against namespaced targets
add_library(lox::common ALIAS lox-common)





# The lexer, parser, etc. frontend
file(GLOB FRONTEND_SOURCES CONFIGURE_DEPENDS frontend/*.cpp)
add_library(lox-frontend STATIC "${FRONTEND_SOURCES}")
target_compile_features(lox-frontend PUBLIC cxx_std_20)
target_include_directories(lox-frontend PUBLIC frontend/)

target_link_libraries(lox-frontend
    PUBLIC
        lox::common
        cxxopts::cxxopts
        Boost::boost
        Boost::container
    PRIVATE
        lox-config
)

add_library(lox::frontend ALIAS lox-frontend)


# Tree walk interpreter backend and executable
file(GLOB TREE_WALKER_SOURCES CONFIGURE_DEPENDS tree-walker/*.cpp)
add_library(tree-walker STATIC "${TREE_WALKER_SOURCES}")
target_compile_features(tree-walker PUBLIC cxx_std_20)
target_include_directories(tree-walker PUBLIC tree-walker/)

target_link_libraries(tree-walker
    PUBLIC
        lox::frontend
        lox::common
    PRIVATE
        lox-config
)

add_library(lox::tree-walker ALIAS tree-walker)



add_executable(lox-twi tree-walker/main.cpp)
target_compile_features(lox-twi PRIVATE cxx_std_20)

target_link_libraries(lox-twi
    PRIVATE
        lox::tree-walker
)






# LLVM based JIT backend and executable
file(GLOB LLVM_JIT_SOURCES CONFIGURE_DEPENDS llvm-jit/*.cpp)
add_library(llvm-jit STATIC "${LLVM_JIT_SOURCES}")
target_compile_features(llvm-jit PUBLIC cxx_std_20)
target_include_directories(llvm-jit PUBLIC llvm-jit/)

target_link_libraries(llvm-jit
    PUBLIC
        lox::frontend
        lox::common
        Boost::boost
        Boost::container
        llvm-dummy
    PRIVATE
        lox-config
)

add_library(lox::llvm-jit ALIAS llvm-jit)



add_executable(lox-jit llvm-jit/main.cpp)
target_compile_features(lox-jit PRIVATE cxx_std_20)

target_link_libraries(lox-jit
    PRIVATE
        lox::llvm-jit
)


