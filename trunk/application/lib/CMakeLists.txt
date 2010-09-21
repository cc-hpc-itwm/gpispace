set (xmls
  4.xml
  5.xml
  6.xml
  dup.xml
  eatN.xml
  make_pair.xml
  sequence_bounded.xml
  sequence_wanted.xml
  sequence.xml
  tagged_sequence_bounded.xml
  tagged_sequence.xml
  trigger_if.xml
  trigger_when.xml
  triple.xml
  wait.xml
  )

install (FILES ${xmls}
  DESTINATION share/sdpa/xml/lib
  COMPONENT examples
  )
