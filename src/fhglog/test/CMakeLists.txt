fhg_add_test (dump_event.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (dump_event.performance.cpp
  BOOST_UNIT_TEST
  PERFORMANCE_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (event.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (format.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (format.performance.cpp
  BOOST_UNIT_TEST
  PERFORMANCE_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (remote_logging.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (level.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)

fhg_add_test (thread_safety.cpp
  BOOST_UNIT_TEST
  PROJECT fhglog
  LINK_LIBRARIES fhglog
)
