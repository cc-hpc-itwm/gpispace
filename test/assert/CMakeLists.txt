gspc_add_test (NAME assert
  SOURCES test.cpp
  USE_BOOST
  COMPILE_FLAGS "-UGSPC_ASSERT_MODE -DGSPC_ASSERT_MODE=1"
  LIBRARIES gspc_util
            gspc_testing
)
