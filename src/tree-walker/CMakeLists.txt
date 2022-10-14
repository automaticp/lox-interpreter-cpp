add_executable(lox-twi main.cpp)

add_library(twi-lib STATIC ExprVisitors.cpp)
target_compile_features(twi-lib PUBLIC cxx_std_20)
target_include_directories(twi-lib PUBLIC .)

target_link_libraries(twi-lib PRIVATE fmt::fmt)

target_link_libraries(lox-twi
    PRIVATE
        twi-lib
        cxxopts::cxxopts
)
