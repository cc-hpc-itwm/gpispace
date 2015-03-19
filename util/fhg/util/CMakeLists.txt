set (srcs
  boost/asio/ip/address.cpp

  backtracing_exception.cpp
  executable_path.cpp
  signal_handler_manager.cpp
  pidfile_writer.cpp
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
  read_file.cpp
  read_lines.cpp

  hostname.cpp

  print_exception.cpp
  system_with_blocked_SIGCHLD.cpp
  wait_and_collect_exceptions.cpp
)

add_library (fhg-util ${srcs})

target_link_libraries (fhg-util
  Util::Generic
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_RANDOM_LIBRARY}
)

install (TARGETS fhg-util
  ARCHIVE DESTINATION lib
)

  add_subdirectory(test)
