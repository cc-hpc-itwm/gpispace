set (files
  Makefile
  get.xpnet
)

install (FILES ${files}
  DESTINATION ${PNETC_EXAMPLES_DIR}/cache/get
  COMPONENT examples
  )
