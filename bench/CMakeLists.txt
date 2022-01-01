cmake_minimum_required(VERSION 3.14)

project(refl-cpp-bench CXX)

if(refl-cpp_EXAMPLES_USE_INSTALLED)
  find_package(refl-cpp REQUIRED)
endif()

set(
    benches
    large-pod
    large-pod-search
)

foreach(bench IN LISTS benches)
  add_executable("${bench}" "bench-${bench}.cpp")
  target_link_libraries("${bench}" PRIVATE refl-cpp::refl-cpp)
  target_compile_features("${bench}" PRIVATE cxx_std_17)
endforeach()
