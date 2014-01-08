fhg_add_test(atomic.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(event.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(queue.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(ini_parser.cpp
  BOOST_UNIT_TEST
  PROJECT util
)

fhg_add_test(assert.cpp
  BOOST_UNIT_TEST
  PROJECT util
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=3 -UFHG_ASSERT_REPLACE_LEGACY -DFHG_ASSERT_REPLACE_LEGACY"
  )

fhg_add_test(counter.cpp
  BOOST_UNIT_TEST
  PROJECT util
)

fhg_add_test(url.cpp
  BOOST_UNIT_TEST
  PROJECT util
)

fhg_add_test(hex.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(pool.cpp
  BOOST_UNIT_TEST
  PROJECT util
  LINK_LIBRARIES fhg-util
)
