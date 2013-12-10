fhg_add_test (evaluate_net.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet ${FhgLog_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${CMAKE_DL_LIBS}
  DESCRIPTION "evaluate various nets and compare the results with given reference nets"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)
