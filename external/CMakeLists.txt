find_package(cxxopts CONFIG REQUIRED)
set_target_properties(cxxopts::cxxopts PROPERTIES IMPORTED_GLOBAL TRUE)

find_package(doctest CONFIG REQUIRED)
set_target_properties(doctest::doctest PROPERTIES IMPORTED_GLOBAL TRUE)
