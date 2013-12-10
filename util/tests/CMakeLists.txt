fhg_add_test(test_semaphore.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(test_atomic.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(test_event.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(test_queue.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
)

fhg_add_test(test_channel.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(test_ini_parser.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
)

fhg_add_test(test_parent_path.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

fhg_add_test(test_assert.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=3 -UFHG_ASSERT_REPLACE_LEGACY -DFHG_ASSERT_REPLACE_LEGACY"
  )

fhg_add_test(test_bool.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(test_counter.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
)

fhg_add_test(test_url.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
)

fhg_add_test(test_hex.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(test_read_show.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test(test_pool.cpp
  BOOST_UNIT_TEST
  PROJECT test_util
  LINK_LIBRARIES fhg-util
)
