set (files
  Makefile
  get.xpnet
)

install (FILES ${files}
  DESTINATION ${PNETC_LIB_DIR}/cache/get
  COMPONENT examples
  )
