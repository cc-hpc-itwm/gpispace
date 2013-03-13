cmake_minimum_required (VERSION "2.6" FATAL_ERROR)

project(gspcnet)
string (TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

include (SDPAFindBoost)
find_package (FhgLog REQUIRED QUIET)
find_package (UTIL REQUIRED QUIET)
find_package (FhgKernel REQUIRED QUIET)
find_package (SDPA REQUIRED QUIET)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(gspc)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()
