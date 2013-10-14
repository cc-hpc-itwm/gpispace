set (gspc_src
  net/io.cpp
  net/util.cpp
  net/error.cpp
  net/limits.cpp
  net/resolver.cpp

  net/frame.cpp
  net/frame_util.cpp
  net/frame_io.cpp
  net/frame_builder.cpp
  net/handle.cpp
  net/serve.cpp
  net/dial.cpp

  net/parse/parser.cpp

  net/common/safe_run_io_service.cpp
  net/common/tcp_connection.cpp
  net/common/unix_connection.cpp

  net/server/queue_manager.cpp
  net/server/service_demux.cpp
  net/server/default_service_demux.cpp
  net/server/tcp_server.cpp
  net/server/unix_server.cpp

  net/client/tcp_client.cpp
  net/client/unix_client.cpp
  net/client/dummy_frame_handler.cpp
  net/client/response.cpp

  net/service/echo.cpp
  net/service/strip_prefix.cpp

  net/auth/cookie.cpp
  net/auth/simple.cpp
  net/auth/default_auth.cpp

  rif/util.cpp
  rif/pipe.cpp
  rif/buffer.cpp
  rif/proc_info.cpp
  rif/process.cpp
  rif/manager.cpp
  rif/convenience.cpp
  rif/null_process_handler.cpp
  rif/supervisor.cpp

  ctl/eval.cpp
  ctl/config.cpp
  ctl/config_cmd.cpp
  ctl/system.cpp

  ctl/log.cpp
  ctl/log_cmd.cpp
)

add_library (libgspc        STATIC ${gspc_src})
add_library (libgspc.shared SHARED ${gspc_src})
target_link_libraries (libgspc ${UTIL_LIBRARIES})
target_link_libraries (libgspc.shared ${UTIL_LIBRARIES})
set_target_properties (libgspc
  PROPERTIES OUTPUT_NAME gspc
             VERSION 0.1
)
set_target_properties (libgspc.shared
  PROPERTIES OUTPUT_NAME gspc
             VERSION 0.1
)

install (TARGETS libgspc ARCHIVE DESTINATION lib COMPONENT libraries)
install (TARGETS libgspc.shared LIBRARY DESTINATION lib COMPONENT runtime-libraries)
