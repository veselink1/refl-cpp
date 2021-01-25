cmake_minimum_required(VERSION 3.14)

project(
    refl-cpp
    VERSION 0.12.1
    DESCRIPTION "A modern compile-time reflection library for C++ with support for overloads, templates, attributes and proxies"
    HOMEPAGE_URL "https://github.com/veselink1/refl-cpp"
    LANGUAGES CXX
)

# ---- Warning guard ----

# Protect dependents from this project's warnings if the guard isn't disabled
set(refl-cpp_warning_guard SYSTEM)
if(refl-cpp_INCLUDE_WITHOUT_SYSTEM)
  set(refl-cpp_warning_guard "")
endif()

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

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(refl-cpp_include_directory "${CMAKE_INSTALL_INCLUDEDIR}/refl-cpp")

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "${refl-cpp_include_directory}"
    COMPONENT refl-cpp_Development
)

install(
    TARGETS refl-cpp
    EXPORT refl-cppTargets
    INCLUDES DESTINATION "${refl-cpp_include_directory}"
)

write_basic_package_version_file(
    refl-cpp-config-version.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

set(refl-cpp_install_cmakedir "${CMAKE_INSTALL_LIBDIR}/cmake/refl-cpp")

install(
    FILES "${PROJECT_BINARY_DIR}/refl-cpp-config-version.cmake"
    DESTINATION "${refl-cpp_install_cmakedir}"
    COMPONENT refl-cpp_Development
)

install(
    EXPORT refl-cppTargets
    FILE refl-cpp-config.cmake
    NAMESPACE refl-cpp::
    DESTINATION "${refl-cpp_install_cmakedir}"
    COMPONENT refl-cpp_Development
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  include(CPack)
endif()
