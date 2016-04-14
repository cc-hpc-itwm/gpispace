fhg_add_test (NAME fhgcom_message
  SOURCES message.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhglog
)

fhg_add_test (NAME fhgcom_p2p
  SOURCES p2p.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhglog
)
