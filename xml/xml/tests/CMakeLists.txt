#bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (parse_duplicate_connect.cpp
  LINK_LIBRARIES xml we
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xpnets
       ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)

if (${Boost_VERSION} EQUAL 104700)
  set (pnet_dir  "${CMAKE_CURRENT_SOURCE_DIR}/pnets/1_47")
else()
  set (pnet_dir  "${CMAKE_CURRENT_SOURCE_DIR}/pnets")
endif()

fhg_add_test (parse_compile_and_compare.cpp
  LINK_LIBRARIES xml we
  DESCRIPTION "pnetc shall produce the same output as given in the reference files"
  ARGS "${CMAKE_CURRENT_SOURCE_DIR}/xpnets"
       "${pnet_dir}"
)

fhg_add_test (util_position.cpp
  LINK_LIBRARIES xml
  DESCRIPTION "test xml_util_position"
)
