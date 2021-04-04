if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "\
FATAL: In-source builds are not supported. \n\
Make a new directory (e.g., 'build/'), and run CMake from there. \
You may need to delete 'CMakeCache.txt' and 'CMakeFiles/'.")
endif()
