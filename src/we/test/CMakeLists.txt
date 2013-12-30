# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (locked_parent_child_relation.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES fhg-util
                 ${Boost_SYSTEM_LIBRARY}
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

fhg_add_test (expr_type_calculate.cpp
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

fhg_add_test (layer_atomics.cpp
  PROJECT we
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
)

fhg_add_test (layer_cancel.cpp
  PROJECT we
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
)

fhg_add_test (port.cpp
  PROJECT we
  LINK_LIBRARIES pnet
)
