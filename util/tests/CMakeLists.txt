# -*- mode: cmake; -*-

fhg_add_test(test_semaphore.cpp
  DESCRIPTION "Tests the semaphore implementation"
)

fhg_add_test(test_event.cpp
  DESCRIPTION "Tests the thread event implementation"
)

fhg_add_test(test_queue.cpp
  DESCRIPTION "Tests the synchronized queue implementation"
)

fhg_add_test(test_channel.cpp
  DESCRIPTION "Tests the channel implementation"
)

fhg_add_test(test_ini_parser.cpp
  DESCRIPTION "Tests the ini-parser"
)

fhg_add_test(test_parent_path.cpp)

fhg_add_test(test_assert.cpp
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=3 -UFHG_ASSERT_REPLACE_LEGACY -DFHG_ASSERT_REPLACE_LEGACY"
  DESCRIPTION "Tests fhg_assert"
  )

fhg_add_test(test_bool.cpp
  LINK_LIBRARIES fhg-util
  DESCRIPTION "Tests the bool_t wrapper around bool"
)

fhg_add_test(test_counter.cpp
  DESCRIPTION "Tests the unique value generating counter"
)

fhg_add_test(test_url.cpp
  DESCRIPTION "Tests the url parser"
)
