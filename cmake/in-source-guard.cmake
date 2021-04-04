if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "\
In-source builds are not supported. \
Delete the generated CMake files and try again with an out-of-source build \
location.")
endif()
