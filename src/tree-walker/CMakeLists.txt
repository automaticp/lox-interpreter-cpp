add_executable(lox-twi main.cpp)
target_compile_features(lox-twi PRIVATE cxx_std_20)
target_include_directories(lox-twi PRIVATE .)
