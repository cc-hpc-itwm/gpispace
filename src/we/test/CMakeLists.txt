# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (layer.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (require_type.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (signature_of.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (exception.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (expr_parser.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
                 ${LibReadline_LIBRARY}
)

fhg_add_test (context.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (activity.cpp
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (execute_activity.cpp
  PROJECT we
  LINK_LIBRARIES pnet
)

add_subdirectory (transition_cannot_fire_when_input_port_is_not_connected)
