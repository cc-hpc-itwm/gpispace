# bundle up

bundle_boost (DESTINATION "external/boost")

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/graphviz/config6"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/bundle/lib/graphviz"
)

file (GLOB GRAPHVIZ_PLUGIN_CORE "${GRAPHVIZ_PLUGIN_DIR}/libgvplugin_core.so*")
file (GLOB GRAPHVIZ_PLUGIN_DOT "${GRAPHVIZ_PLUGIN_DIR}/libgvplugin_dot_layout.so*")
install (PROGRAMS ${GRAPHVIZ_PLUGIN_CORE} ${GRAPHVIZ_PLUGIN_DOT}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec/bundle/lib/graphviz"
)
