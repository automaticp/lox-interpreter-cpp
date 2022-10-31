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
file(GLOB TREE_WALKER_SOURCES CONFIGURE_DEPENDS tree-walker/*.cpp)
add_executable(lox-twi "${TREE_WALKER_SOURCES}")
target_compile_features(lox-twi PUBLIC cxx_std_20)
target_include_directories(lox-twi PUBLIC tree-walker/)

target_link_libraries(lox-twi
    PRIVATE
        lox::frontend
        lox::common
        lox::shared
        cxxopts::cxxopts
)
