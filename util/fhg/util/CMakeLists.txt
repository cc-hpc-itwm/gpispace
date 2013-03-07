# -*- mode: cmake; -*-

# check for 'pthread_setname_np'
include(CheckCSourceCompiles)
find_package (Threads)
set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
check_c_source_compiles("
#include <pthread.h>
int main () { pthread_setname_np (pthread_self (), \"test\"); }
" HAVE_PTHREAD_SETNAME)

if (HAVE_PTHREAD_SETNAME)
  add_definitions (-DHAVE_PTHREAD_SETNAME)
endif()

set (hdrs
  join.hpp
  read_bool.hpp
  read.hpp
  show.hpp
  split.hpp
  starts_with.hpp
  warnings.hpp

  filesystem.hpp
  cpp.hpp

  ini-parser.hpp
  ini-parser.tcc
  ini-parser-except.hpp
  ini-parser-helper.hpp

  getenv.hpp

  xml.hpp
  bool.hpp
  bool_io.hpp

  random.hpp

  counter.hpp
  backtracing_exception.hpp
  backtracing_exception.fwd.hpp

  url.hpp
  setproctitle.h
  threadname.hpp

  remove_prefix.hpp

  stat.hpp
)

set (srcs
  backtracing_exception.cpp
  setproctitle.c
  threadname.cpp
  remove_prefix.cpp
  read_bool.cpp
  stat.cpp

  thread/poll.cpp
  parse/position.cpp
)

add_library (fhg-util ${srcs})
add_library (fhg-util-shared SHARED ${srcs})
set_target_properties(fhg-util-shared PROPERTIES OUTPUT_NAME fhg-util)

install(FILES ${hdrs}
  DESTINATION include/fhg/util
  COMPONENT headers)
install (TARGETS fhg-util
  ARCHIVE DESTINATION lib
  COMPONENT runtime
)
install (TARGETS fhg-util-shared
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

add_subdirectory (boost)
add_subdirectory(parse)
add_subdirectory(thread)
add_subdirectory(filesystem)
add_subdirectory(cpp)
if (BUILD_TESTING)
  add_subdirectory(test)
endif ()
