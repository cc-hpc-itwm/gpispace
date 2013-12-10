# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (require_type.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (signature_of.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (exception.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (expr_type_calculate.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (expr_parser.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES pnet
                 ${LibReadline_LIBRARY}
)

fhg_add_test (context.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (activity.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (execute_activity.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (layer_atomics.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
)

fhg_add_test (layer_cancel.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
)

fhg_add_test (port.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
)
