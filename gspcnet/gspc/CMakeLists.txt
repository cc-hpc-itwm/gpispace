set (gspcnet_src
  net/frame.cpp
  net/frame_io.cpp

  net/parse/parser.cpp

  net/server/queue_manager.cpp
  net/server/service_demux.cpp
  net/server/default_service_demux.cpp
)

add_library (gspcnet        STATIC ${gspcnet_src})
add_library (gspcnet.shared SHARED ${gspcnet_src})
set_target_properties (gspcnet.shared
  PROPERTIES OUTPUT_NAME gspcnet
)
