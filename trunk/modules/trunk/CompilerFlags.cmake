if (${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions("-DNDEBUG")
endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Wno-non-virtual-dtor -Wreturn-type -Wno-system-headers")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wmissing-include-dirs -Wno-pragmas -Wredundant-decls")
  # produces a lot of warnings with (at least) boost 1.38:
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-default -Wfloat-equal")

  # release flags
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")

  # debug flags
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -fno-omit-frame-pointer")
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")

  # gprof and gcov support
  set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers -pg")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -fprofile-arcs -ftest-coverage")
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

# TODO: we need to check the compiler here, gcc does not know about those flags, is this The Right Thing To Do (TM)?
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
  set(CMAKE_CXX_FLAGS "-wd383 -wd981")
  message(STATUS "compiler: ${CMAKE_CXX_COMPILER_MAJOR}.${CMAKE_CXX_COMPILER_MINOR}")
  if (${CMAKE_CXX_COMPILER_MAJOR} GREATER 9)
    message(STATUS "Warning: adding __aligned__=ignored to the list of definitions")
    add_definitions("-D__aligned__=ignored")
  endif (${CMAKE_CXX_COMPILER_MAJOR} GREATER 9)
endif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
