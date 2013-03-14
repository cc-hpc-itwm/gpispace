set (gspcnet_src
  net/parse/parser.cpp
  net/frame.cpp
  net/frame_io.cpp
)

add_library (gspcnet        STATIC ${gspcnet_src})
add_library (gspcnet.shared SHARED ${gspcnet_src})
set_target_properties (gspcnet.shared
  PROPERTIES OUTPUT_NAME gspcnet
)
