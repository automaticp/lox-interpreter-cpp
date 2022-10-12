add_executable(lox-twi main.cpp)

add_library(twi-lib INTERFACE)
target_compile_features(twi-lib INTERFACE cxx_std_20)
target_include_directories(twi-lib INTERFACE .)

target_link_libraries(lox-twi
    PRIVATE
        twi-lib
        cxxopts::cxxopts
)
