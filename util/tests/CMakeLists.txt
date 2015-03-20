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
