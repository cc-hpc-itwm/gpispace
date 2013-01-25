set (files
  Makefile
  demo.xpnet
)

install (FILES ${files}
  DESTINATION ${PNETC_EXAMPLES_DIR}/cache/demo
  COMPONENT examples
)
