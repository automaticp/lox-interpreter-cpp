file(GLOB twi_test_sources CONFIGURE_DEPENDS tree-walker/*.test.cpp)
add_executable(twi-tests ${twi_test_sources})
target_link_libraries(twi-tests PRIVATE lox::frontend doctest::doctest)
