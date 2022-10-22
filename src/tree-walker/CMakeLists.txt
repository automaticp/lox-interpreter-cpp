add_executable(lox-twi main.cpp)

add_library(twi-lib
    STATIC
        ExprVisitors.cpp
        ExprInterpreterVisitor.cpp
        StmtVisitors.cpp
        Value.cpp
        Builtins.cpp
        Environment.cpp
)
target_compile_features(twi-lib PUBLIC cxx_std_20)
target_include_directories(twi-lib PUBLIC .)

target_link_libraries(twi-lib PRIVATE fmt::fmt Boost::container Boost::boost)

target_link_libraries(lox-twi
    PRIVATE
        twi-lib
        cxxopts::cxxopts
)
