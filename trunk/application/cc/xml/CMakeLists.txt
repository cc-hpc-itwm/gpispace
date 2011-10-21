include(pnet_macros)

configure_file(compile.xml.in ${CMAKE_CURRENT_BINARY_DIR}/compile.xml @ONLY)

pnet_compile(sdpacc ${CMAKE_CURRENT_BINARY_DIR}/compile.xml
  OUTPUT compile.pnet GENERATE gen-sdpacc
  INSTALL share/sdpa/cc
#  BUILD
)
