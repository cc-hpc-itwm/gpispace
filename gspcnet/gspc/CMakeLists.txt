set (gspc_common_src
  net/parse/parser.cpp
  net/frame.cpp
  net/frame_io.cpp
)

add_library (gspcnet-common        STATIC ${gspc_common_src})
add_library (gspcnet-common.shared SHARED ${gspc_common_src})
set_target_properties (gspcnet-common.shared
  PROPERTIES OUTPUT_NAME gspcnet-common
)
