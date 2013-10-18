# -*- mode: cmake; -*-

# check for 'pthread_setname_np'
include(CheckCSourceCompiles)
find_package (Threads QUIET)
set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
check_c_source_compiles("
#include <pthread.h>
int main () { pthread_setname_np (pthread_self (), \"test\"); }
" HAVE_PTHREAD_SETNAME)

if (HAVE_PTHREAD_SETNAME)
  add_definitions (-DHAVE_PTHREAD_SETNAME)
endif()

set (hdrs
  alphanum.hpp

  join.hpp
  read_bool.hpp
  read.hpp
  show.hpp
  split.hpp
  starts_with.hpp
  warnings.hpp

  filesystem.hpp

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
  now.hpp
  get_home_dir.hpp
  hex.hpp
  read_file.hpp

  get_cpucount.h
)

set (srcs
  backtracing_exception.cpp
  setproctitle.c
  program_info.c
  threadname.cpp
  remove_prefix.cpp
  read_bool.cpp
  now.cpp
  stat.cpp

  ostream_modifier.cpp

  thread/poll.cpp
  thread/pool.cpp

  parse/position.cpp
  parse/error.cpp
  parse/require.cpp

  num.cpp
  num/show.cpp
  indenter.cpp
  cpp/block.cpp
  cpp/include.cpp
  cpp/include_guard.cpp
  cpp/namespace.cpp
  cpp/struct.cpp

  xml.cpp
  get_home_dir.cpp
  hex.cpp
  read_file.cpp

  get_cpucount.c
)

add_library (fhg-util ${srcs})
add_library (fhg-util-shared SHARED ${srcs})
set_target_properties(fhg-util-shared
  PROPERTIES OUTPUT_NAME fhg-util
             VERSION     1
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util
  COMPONENT runtime-headers)
install (TARGETS fhg-util
  ARCHIVE DESTINATION lib
  COMPONENT runtime
)
install (TARGETS fhg-util-shared
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

add_subdirectory (boost)
add_subdirectory(thread)
add_subdirectory(filesystem)
if (BUILD_TESTING)
  add_subdirectory(test)
endif ()
