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





# The lexer, parser, etc. frontend
file(GLOB FRONTEND_SOURCES CONFIGURE_DEPENDS frontend/*.cpp)
add_library(lox-frontend STATIC "${FRONTEND_SOURCES}")
target_compile_features(lox-frontend PUBLIC cxx_std_20)
target_include_directories(lox-frontend PUBLIC frontend/)

target_link_libraries(lox-frontend
    PUBLIC
        lox::common
        Boost::boost
        Boost::container
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
)

add_library(lox::tree-walker ALIAS tree-walker)



add_executable(lox-twi tree-walker/main.cpp)
target_compile_features(lox-twi PRIVATE cxx_std_20)

target_link_libraries(lox-twi
    PRIVATE
        lox::tree-walker
        cxxopts::cxxopts
)

