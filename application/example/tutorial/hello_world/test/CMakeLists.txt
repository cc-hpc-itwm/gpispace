if (BUILD_TESTING)
  configure_file (test.sh.in test.sh @ONLY)

  add_test (
    "tutorial_hello_world"
    sh ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  )
endif()
