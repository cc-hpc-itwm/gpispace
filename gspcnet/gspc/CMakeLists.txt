set (gspcnet_src
  net/util.cpp
  net/error.cpp
  net/limits.cpp
  net/resolver.cpp

  net/frame.cpp
  net/frame_util.cpp
  net/frame_io.cpp
  net/frame_buffer.cpp
  net/frame_builder.cpp
  net/handle.cpp
  net/serve.cpp
  net/dial.cpp

  net/parse/parser.cpp

  net/server/queue_manager.cpp
  net/server/service_demux.cpp
  net/server/default_service_demux.cpp
  net/server/tcp_server.cpp
  net/server/tcp_connection.cpp
  net/server/unix_server.cpp
  net/server/unix_connection.cpp

  net/client/tcp_client.cpp
  net/client/unix_client.cpp
  net/client/dummy_frame_handler.cpp

  net/service/echo.cpp
  net/service/strip_prefix.cpp
)

add_library (gspcnet        STATIC ${gspcnet_src})
add_library (gspcnet.shared SHARED ${gspcnet_src})
set_target_properties (gspcnet.shared
  PROPERTIES OUTPUT_NAME gspcnet
)
