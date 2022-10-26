find_package(cxxopts CONFIG REQUIRED)
set_target_properties(cxxopts::cxxopts PROPERTIES IMPORTED_GLOBAL TRUE)

find_package(doctest CONFIG REQUIRED)
set_target_properties(doctest::doctest PROPERTIES IMPORTED_GLOBAL TRUE)

find_package(fmt CONFIG REQUIRED)
set_target_properties(fmt::fmt PROPERTIES IMPORTED_GLOBAL TRUE)

set(Boost_NO_WARN_NEW_VERSIONS TRUE)
find_package(Boost REQUIRED COMPONENTS container)
set_target_properties(Boost::boost Boost::container PROPERTIES IMPORTED_GLOBAL TRUE)
