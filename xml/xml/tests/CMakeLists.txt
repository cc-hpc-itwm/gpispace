#bernd.loerwald@itwm.fraunhofer.de

include (fhg_test_macros)

fhg_add_test (parse_duplicate_connect.cpp
  LINK_LIBRARIES xml_parse ${WE_LIBRARIES}
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xpnets
       ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)

fhg_add_test (parse_compile_and_compare.cpp
  LINK_LIBRARIES xml_parse ${WE_LIBRARIES}
  DESCRIPTION "pnetc shall produce the same output as given in the reference files"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xpnets
       ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)
