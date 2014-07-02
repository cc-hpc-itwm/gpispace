set (srcs
  ../syscall.cpp

  backtracing_exception.cpp
  daemonize.cpp
  signal_handler_manager.cpp
  pidfile_writer.cpp
  program_info.c
  random_string.cpp
  remove_prefix.cpp
  read_bool.cpp
  now.cpp

  ostream_modifier.cpp

  parse/position.cpp
  parse/error.cpp
  parse/require.cpp
  parse/until.cpp

  num.cpp
  num/show.cpp
  indenter.cpp
  cpp/block.cpp
  cpp/include.cpp
  cpp/include_guard.cpp
  cpp/namespace.cpp
  cpp/struct.cpp

  xml.cpp
  hex.cpp
  read_file.cpp

  hostname.cpp
  environment.cpp
)

add_library (fhg-util ${srcs})
add_library (fhg-util-shared SHARED ${srcs})
set_target_properties(fhg-util-shared
  PROPERTIES OUTPUT_NAME fhg-util
             VERSION     1
)

target_link_libraries (fhg-util
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_RANDOM_LIBRARY}
)
target_link_libraries (fhg-util-shared
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_RANDOM_LIBRARY}
  )

install (TARGETS fhg-util
  ARCHIVE DESTINATION lib
)
install (TARGETS fhg-util-shared
  LIBRARY DESTINATION lib
)

  add_subdirectory(test)
