# -*- mode: cmake; -*-

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions("-DNDEBUG")
endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

if(NOT WIN32)
if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  include (CheckCXXSourceCompiles)

  set(CMAKE_CXX_FLAGS "${CXXFLAGS} -fpic -fPIC -W -Wall -Wextra -Wnon-virtual-dtor -Wno-system-headers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wmissing-include-dirs -Wno-pragmas -Wredundant-decls")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-redundant-decls")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-default")

  set (CMAKE_REQUIRED_FLAGS "-Wno-ignored-qualifiers")
  set (__CXX_FLAG_CHECK_SOURCE "int main () { return 0; }")
  message(STATUS "checking if ${CMAKE_REQUIRED_FLAGS} works")
  CHECK_CXX_SOURCE_COMPILES ("${__CXX_FLAG_CHECK_SOURCE}" W_NO_IGNORED_QUALIFIERS)
  set (CMAKE_REQUIRED_FLAGS)
  if (W_NO_IGNORED_QUALIFIERS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
  endif (W_NO_IGNORED_QUALIFIERS)

  # release flags
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-unused-parameter -fstack-protector-all")

  # debug flags
  set(CMAKE_CXX_FLAGS_DEBUG
      "-O0 -g -ggdb -fno-omit-frame-pointer -Woverloaded-virtual -Wno-system-headers"
     )
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")

  # gprof and gcov support
  set(CMAKE_CXX_FLAGS_PROFILE
      "-O0 -fprofile-arcs -ftest-coverage -pg -g -ggdb -Wreturn-type -Woverloaded-virtual -Wno-system-headers"
      CACHE STRING "Flags for Profile build"
     )
endif ()
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CXXFLAGS} -fPIC -W -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-constant-logical-operand")
endif()

# TODO: we need to check the compiler here, gcc does not know about those flags, is this The Right Thing To Do (TM)?
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
  set(CMAKE_CXX_FLAGS "${CXXFLAGS} -wd383 -wd981")
  message(STATUS "compiler: ${CMAKE_CXX_COMPILER_MAJOR}.${CMAKE_CXX_COMPILER_MINOR}")
  if (${CMAKE_CXX_COMPILER_MAJOR} GREATER 9)
    message(STATUS "Warning: adding __aligned__=ignored to the list of definitions")
    add_definitions("-D__aligned__=ignored")
  endif (${CMAKE_CXX_COMPILER_MAJOR} GREATER 9)
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
else(NOT WIN32)
#  set(CMAKE_CXX_FLAGS "${CXXFLAGS} -W -Wall -Wextra -Wnon-virtual-dtor -Wno-system-headers")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wmissing-include-dirs -Wno-pragmas -Wredundant-decls")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS " /DWIN32 /D_WINDOWS /W3 /Zm1000 /EHsc /GR ")
  set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /D NDEBUG")
endif(NOT WIN32)
