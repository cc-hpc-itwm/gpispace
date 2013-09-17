set (gspcnet_src
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
)

add_library (gspcnet        STATIC ${gspcnet_src})
add_library (gspcnet.shared SHARED ${gspcnet_src})
target_link_libraries (gspcnet ${UTIL_LIBRARIES})
target_link_libraries (gspcnet.shared ${UTIL_LIBRARIES})
set_target_properties (gspcnet.shared
  PROPERTIES OUTPUT_NAME gspcnet
             VERSION 0.1
)

install (TARGETS gspcnet ARCHIVE DESTINATION lib COMPONENT libraries)
install (TARGETS gspcnet.shared LIBRARY DESTINATION lib COMPONENT runtime-libraries)
