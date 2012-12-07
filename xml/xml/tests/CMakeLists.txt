#bernd.loerwald@itwm.fraunhofer.de

include (fhg_test_macros)

set (xpnet_files
  "connect_in_out_same.xpnet"
  "connect_read_out_same.xpnet"
  "connect_in_read_same.xpnet"
  "connect_in_in_same.xpnet"
  "connect_read_read_same.xpnet"
  "connect_out_out_same.xpnet"
)

foreach (file ${xpnet_files})
  configure_file (${CMAKE_CURRENT_SOURCE_DIR}/xpnets/${file}
                  ${CMAKE_CURRENT_BINARY_DIR}/${file}
                  COPYONLY)
endforeach()

fhg_add_test (parse_duplicate_connect.cpp
  LINK_LIBRARIES xml_parse ${WE_LIBRARIES}
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
  ARGS ${CMAKE_CURRENT_BINARY_DIR}
)
