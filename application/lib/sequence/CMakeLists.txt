set (xmls
  ntom.xpnet
  interval.xpnet
  )

install (FILES ${xmls}
  DESTINATION ${PNETC_LIB_DIR}/sequence
  COMPONENT examples
  )
