set (gspcctl_src
  ctl/eval.cpp
)

add_library (gspcctl        STATIC ${gspcctl_src})
add_library (gspcctl.shared SHARED ${gspcctl_src})
set_target_properties (gspcctl.shared
  PROPERTIES OUTPUT_NAME gspcctl
             VERSION 0.1
)

install (TARGETS gspcctl ARCHIVE DESTINATION lib COMPONENT libraries)
install (TARGETS gspcctl.shared LIBRARY DESTINATION lib COMPONENT runtime-libraries)
