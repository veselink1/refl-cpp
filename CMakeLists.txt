cmake_minimum_required(VERSION 3.14)

project(
    refl-cpp
    VERSION 0.12.1
    DESCRIPTION "A modern compile-time reflection library for C++ with support for overloads, templates, attributes and proxies"
    HOMEPAGE_URL "https://github.com/veselink1/refl-cpp"
    LANGUAGES CXX
)

include(cmake/variables.cmake)

# ---- Declare library ----

add_library(refl-cpp INTERFACE)
add_library(refl-cpp::refl-cpp ALIAS refl-cpp)

target_include_directories(
    refl-cpp
    ${refl-cpp_warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(refl-cpp INTERFACE cxx_std_17)

# ---- Install ----

include(cmake/install-rules.cmake)

# ---- Developer mode ----

if(NOT refl-cpp_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(AUTHOR_WARNING "Developer mode is for developers of refl-cpp")
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

option(BUILD_EXAMPLES "Build the examples tree." ON)
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
