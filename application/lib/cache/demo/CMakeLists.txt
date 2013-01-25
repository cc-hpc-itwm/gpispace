set (files
  Makefile
  demo.xpnet
)

install (FILES ${files}
  DESTINATION ${PNETC_LIB_DIR}/cache/demo
  COMPONENT examples
)
