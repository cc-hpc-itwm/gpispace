set (xmls
  4.xml
  5.xml
  6.xml
  dup.xml
  eatN.xml
  make_pair.xml
  replicate.xpnet
  sequence_bounded.xml
  sequence.xml
  sequence_control.xml
  tagged_sequence_bounded.xml
  tagged_sequence.xml
  trigger_if.xml
  trigger_when.xml
  triple.xml
  wait.xml
  )

install (FILES ${xmls}
  DESTINATION ${PNETC_LIB_DIR}
  COMPONENT examples
  )

add_subdirectory (sequence)
add_subdirectory (point)
add_subdirectory (cache)
