string(
    COMPARE EQUAL
    "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
    PROJECT_IS_TOP_LEVEL
)

# ---- Developer mode ----

if(PROJECT_IS_TOP_LEVEL)
  option(refl-cpp_DEVELOPER_MODE "Enable developer mode" OFF)

  if("$ENV{CI}")
    set(refl-cpp_DEVELOPER_MODE ON CACHE INTERNAL "")
  endif()
endif()

# ---- Warning guard ----

# Protect dependents from this project's warnings if the guard isn't disabled
set(refl-cpp_warning_guard SYSTEM)
if(PROJECT_IS_TOP_LEVEL OR refl-cpp_INCLUDE_WITHOUT_SYSTEM)
  set(refl-cpp_warning_guard "")
endif()
