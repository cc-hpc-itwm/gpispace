#bernd.loerwald@itwm.fraunhofer.de

extended_add_library (NAME "${LIB_PREFIX}graphviz-wrapper"
  SOURCES "graphviz.cpp"
  LIBRARIES ${GRAPHVIZ_LIBRARIES}
)
#!\todo move into extended_add_library
target_include_directories ("${LIB_PREFIX}graphviz-wrapper"
  SYSTEM PRIVATE "${GRAPHVIZ_INCLUDE_DIR}"
)
