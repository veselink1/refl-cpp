cmake_minimum_required(VERSION 3.14)

project(refl-cpp-tests CXX)

# ---- Dependencies ----

set(refl-cpp_INCLUDE_WITHOUT_SYSTEM YES CACHE INTERNAL "")

include(FetchContent)
FetchContent_Declare(refl-cpp SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
FetchContent_MakeAvailable(refl-cpp)

# ---- Test ----

enable_testing()

set(
    sources
    010-TestCase.cpp
    020-ConstString.hpp
    030-TypeList.hpp
    040-TypeTraits.hpp
    050-Attributes.hpp
    060-Utils.hpp
    070-Descriptors.hpp
    080-Runtime.hpp
    090-StdTypes.hpp
    extern/catch2/catch.hpp
)

add_executable(refl-cpp-test ${sources})
target_link_libraries(refl-cpp-test PRIVATE refl-cpp::refl-cpp)
target_compile_features(refl-cpp-test PRIVATE cxx_std_17)
target_include_directories(refl-cpp-test PRIVATE "${PROJECT_SOURCE_DIR}")

add_test(NAME all_tests COMMAND refl-cpp-test)
