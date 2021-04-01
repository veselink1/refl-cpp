cmake_minimum_required(VERSION 3.14)

project(refl-cpp-examples CXX)

if(refl-cpp_EXAMPLES_USE_INSTALLED)
  find_package(refl-cpp REQUIRED)
endif()

set(
    examples
    binding
    builders
    custom-rtti
    dao
    inheritance
    # macro
    partials
    proxy
    serialization
    struct-of-arrays
)

foreach(example IN LISTS examples)
  add_executable("${example}" "example-${example}.cpp")
  target_link_libraries("${example}" PRIVATE refl-cpp::refl-cpp)
  target_compile_features("${example}" PRIVATE cxx_std_17)
endforeach()
