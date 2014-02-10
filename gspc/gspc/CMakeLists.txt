# json has a lot that are used in debug only
add_definitions (-Wno-unused-parameter)

set (gspc_src
  net/constants.cpp
  net/io.cpp
  net/util.cpp
  net/error.cpp
  net/limits.cpp
  net/resolver.cpp

  net/frame.cpp
  net/frame_builder.cpp
  net/serve.cpp
  net/dial.cpp

  net/parse/parser.cpp

  net/common/tcp_connection.cpp
  net/common/unix_connection.cpp

  net/server/queue_manager.cpp
  net/server/service_demux.cpp
  net/server/default_service_demux.cpp
  net/server/default_queue_manager.cpp
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

  kvs/api.cpp
  kvs/kvs.cpp
  kvs/impl/kvs_impl.cpp
  kvs/impl/kvs_net_frontend.cpp
  kvs/impl/kvs_net_service.cpp

  rif/daemon.cpp
  rif/util.cpp
  rif/pipe.cpp
  rif/buffer.cpp
  rif/proc_info.cpp
  rif/process.cpp
  rif/manager.cpp
  rif/null_process_handler.cpp
  rif/supervisor.cpp

  ctl/exit_code.cpp
  ctl/eval.cpp
  ctl/config.cpp
  ctl/config_cmd.cpp
  ctl/system.cpp
  ctl/session.cpp

  ctl/log_cmd.cpp
)

add_library (libgspc        STATIC ${gspc_src})
add_library (libgspc.shared SHARED ${gspc_src})
target_link_libraries (libgspc ${UTIL_LIBRARIES} ${Boost_REGEX_LIBRARY})
target_link_libraries (libgspc.shared ${UTIL_LIBRARIES} ${Boost_REGEX_LIBRARY})
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

install (FILES
  drts/context_fwd.hpp
  drts/context.hpp

  DESTINATION include/gspc/drts
  COMPONENT sdk
)
