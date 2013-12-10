fhg_add_test(semaphore.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(atomic.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(event.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(queue.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(channel.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(ini_parser.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
)

fhg_add_test(parent_path.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

fhg_add_test(assert.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=3 -UFHG_ASSERT_REPLACE_LEGACY -DFHG_ASSERT_REPLACE_LEGACY"
  )

fhg_add_test(bool.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(counter.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
)

fhg_add_test(url.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
)

fhg_add_test(hex.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(read_show.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(pool.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)
