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
)
# Link against namespaced targets
add_library(lox::common ALIAS lox-common)



# Common classes that cannot be separated between backends
# Ideally, should not exist
file(GLOB LOX_SHARED_SOURCES CONFIGURE_DEPENDS lox-shared/*.cpp)
add_library(lox-shared STATIC "${LOX_SHARED_SOURCES}")
target_compile_features(lox-shared PUBLIC cxx_std_20)
target_include_directories(lox-shared PUBLIC lox-shared/)

target_link_libraries(lox-shared
    PUBLIC
        lox::common
        fmt::fmt
)

add_library(lox::shared ALIAS lox-shared)



# The lexer, parser, etc. frontend
file(GLOB FRONTEND_SOURCES CONFIGURE_DEPENDS frontend/*.cpp)
add_library(lox-frontend STATIC "${FRONTEND_SOURCES}")
target_compile_features(lox-frontend PUBLIC cxx_std_20)
target_include_directories(lox-frontend PUBLIC frontend/)

target_link_libraries(lox-frontend
    PUBLIC
        lox::common
        lox::shared
        Boost::boost
        Boost::container
)

add_library(lox::frontend ALIAS lox-frontend)


# Tree walk interpreter backend and executable
add_library(tree-walker INTERFACE)
target_compile_features(tree-walker INTERFACE cxx_std_20)
target_include_directories(tree-walker INTERFACE tree-walker/)

target_link_libraries(tree-walker
    INTERFACE
        lox::frontend
        lox::common
        lox::shared
)

add_library(lox::tree-walker ALIAS tree-walker)

add_executable(lox-twi tree-walker/main.cpp)
target_compile_features(lox-twi PRIVATE cxx_std_20)

target_link_libraries(lox-twi
    PRIVATE
        tree-walker
        cxxopts::cxxopts
)


# The real bad:
# lox-shared needs Interpreter from the tree-walker backend,
# in particular, '...InterpreterVisitor's depend on it.
# And I can't make them part of tree-walker backend, since
# they constitute a set of visitors for IExpr and IStmt, and
# must be in lox-shared.
target_link_libraries(lox-shared PRIVATE lox::tree-walker)


# Another bad, similar story:
# Value's Function needs FunStmt from lox-shared.
# IExpr and IStmt concrete types should be part of
# lox-common, ideally, but can't as they depend
# on their Visitors.
target_link_libraries(lox-common PRIVATE lox::shared)

# The linking is PRIVATE, so it sorta works,
# but it's a tangled spaghetti mess nonetheless.
# All of this hopefully will be resolved later.
# I have just the thing to make this happen...
