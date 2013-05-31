qt4_wrap_cpp (wrapped_cpp_server mon/server.hpp)

set (gspcmon_src
  mon/server.cpp
  ${wrapped_cpp_server}
)

add_library (gspcmon        STATIC ${gspcmon_src})
add_library (gspcmon.shared SHARED ${gspcmon_src})
set_target_properties (gspcmon.shared
  PROPERTIES OUTPUT_NAME gspcmon
             VERSION 0.1
)

install (TARGETS gspcmon ARCHIVE DESTINATION lib COMPONENT libraries)
install (TARGETS gspcmon.shared LIBRARY DESTINATION lib COMPONENT libraries)
