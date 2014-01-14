# check for 'pthread_setname_np'
include(CheckCSourceCompiles)
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
  daemonize.hpp

  join.hpp
  read_bool.hpp
  split.hpp
  starts_with.hpp

  ini-parser.hpp
  ini-parser.tcc
  ini-parser-except.hpp
  ini-parser-helper.hpp

  getenv.hpp

  xml.hpp

  counter.hpp
  backtracing_exception.hpp
  backtracing_exception.fwd.hpp

  url.hpp
  threadname.hpp

  remove_prefix.hpp

  now.hpp
  get_home_dir.hpp
  hex.hpp
  read_file.hpp

  get_cpucount.h
)

set (srcs
  backtracing_exception.cpp
  daemonize.cpp
  program_info.c
  threadname.cpp
  remove_prefix.cpp
  read_bool.cpp
  now.cpp

  ostream_modifier.cpp

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

  hostname.cpp
)

add_library (fhg-util ${srcs})
add_library (fhg-util-shared SHARED ${srcs})
set_target_properties(fhg-util-shared
  PROPERTIES OUTPUT_NAME fhg-util
             VERSION     1
)

target_link_libraries (fhg-util ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (fhg-util-shared ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

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
if (BUILD_TESTING)
  add_subdirectory(test)
endif ()
