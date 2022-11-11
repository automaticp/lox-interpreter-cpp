find_package(cxxopts CONFIG REQUIRED)
set_target_properties(cxxopts::cxxopts PROPERTIES IMPORTED_GLOBAL TRUE)

find_package(doctest CONFIG REQUIRED)
set_target_properties(doctest::doctest PROPERTIES IMPORTED_GLOBAL TRUE)

find_package(fmt CONFIG REQUIRED)
set_target_properties(fmt::fmt PROPERTIES IMPORTED_GLOBAL TRUE)

set(Boost_NO_WARN_NEW_VERSIONS TRUE)
find_package(Boost REQUIRED COMPONENTS container)
set_target_properties(Boost::boost Boost::container PROPERTIES IMPORTED_GLOBAL TRUE)




find_package(Clang CONFIG REQUIRED)
message(STATUS "Clang_FOUND = ${Clang_FOUND}")
message(STATUS "Clang_DIR = ${Clang_DIR}")
# set(LLVM_USE_SANITIZER "Address;Undefined")
find_package(LLVM CONFIG REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(HandleLLVMOptions)

add_library(llvm-dummy INTERFACE)
message(STATUS "LLVM_DEFINITIONS = ${LLVM_DEFINITIONS}")
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
message(STATUS "LLVM_DEFINITIONS_LIST = ${LLVM_DEFINITIONS_LIST}")
message(STATUS "LLVM_INCLUDE_DIRS = ${LLVM_INCLUDE_DIRS}")
target_compile_definitions(llvm-dummy INTERFACE ${LLVM_DEFINITIONS})
target_include_directories(llvm-dummy SYSTEM INTERFACE ${LLVM_INCLUDE_DIRS})

message(STATUS "LLVM_ENABLE_PROJECTS = ${LLVM_ENABLE_PROJECTS}")

# add_definitions(${LLVM_DEFINITIONS})

# target_include_directories(main PRIVATE ${LLVM_INCLUDE_DIRS})

# Find the libraries that correspond to the LLVM components that we wish to use
llvm_map_components_to_libnames(llvm_libs Support Core IRReader)
# llvm_map_components_to_libnames(llvm_libs Clang All)

target_link_libraries(llvm-dummy
    INTERFACE
        libclang
        ${llvm_libs}
)


message(STATUS "llvm_libs = ${llvm_libs}")
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
