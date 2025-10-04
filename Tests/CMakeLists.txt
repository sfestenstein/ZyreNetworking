
find_package(GTest QUIET)

if (NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
  )

  # Ensure MSVC uses shared CRT when requested by project
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(BUILD_GTEST ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)
endif()

# Prefer using the found GTest (or the one we fetched) in subdirs
add_subdirectory(CommonUtils)

