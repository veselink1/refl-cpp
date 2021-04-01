# ---- Warning guard ----

# Protect dependents from this project's warnings if the guard isn't disabled
set(refl-cpp_warning_guard SYSTEM)
if(PROJECT_IS_TOP_LEVEL OR refl-cpp_INCLUDE_WITHOUT_SYSTEM)
  set(refl-cpp_warning_guard "")
endif()
