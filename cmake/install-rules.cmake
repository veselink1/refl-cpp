if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/refl-cpp CACHE PATH "")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT refl-cpp_Development
)

install(
    TARGETS refl-cpp
    EXPORT refl-cppTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    refl-cpp-config-version.cmake
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

set(
    refl-cpp_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/refl-cpp"
    CACHE STRING "CMake package config location relative to the install prefix"
)
mark_as_advanced(refl-cpp_INSTALL_CMAKEDIR)

install(
    FILES "${PROJECT_BINARY_DIR}/refl-cpp-config-version.cmake"
    DESTINATION "${refl-cpp_INSTALL_CMAKEDIR}"
    COMPONENT refl-cpp_Development
)

install(
    EXPORT refl-cppTargets
    FILE refl-cpp-config.cmake
    NAMESPACE refl-cpp::
    DESTINATION "${refl-cpp_INSTALL_CMAKEDIR}"
    COMPONENT refl-cpp_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
