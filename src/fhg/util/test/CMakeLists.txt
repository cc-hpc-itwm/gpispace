# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (alphanum.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (num.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (remove_prefix.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (xml_stream.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(event.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES Boost::thread Boost::system fhg-util
)

fhg_add_test(queue.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES Boost::thread Boost::system fhg-util
)

fhg_add_test(assert.cpp
  BOOST_UNIT_TEST
  PROJECT util
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=1"
  LINK_LIBRARIES fhg-util
  )

fhg_add_test(counter.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(bounded_queue.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES Boost::thread Boost::system fhg-util
)
