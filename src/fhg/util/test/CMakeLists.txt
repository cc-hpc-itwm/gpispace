fhg_add_test (NAME util_num
  SOURCES num.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_remove_prefix
  SOURCES remove_prefix.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_xml_stream
  SOURCES xml_stream.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_assert
  SOURCES assert.cpp
  USE_BOOST
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=1"
  LIBRARIES fhg-util
  )

fhg_add_test (NAME util_signal_handler_Execution_can_be_stopped_manually
  SOURCES "signal_handler_Execution_can_be_stopped_manually.cpp"
  LIBRARIES fhg-util
)
fhg_add_test (NAME util_signal_handler_Execution_can_be_stopped_by_SIGTERM
  SOURCES "signal_handler_Execution_can_be_stopped_by_SIGTERM.cpp"
  LIBRARIES fhg-util
)
fhg_add_test (NAME util_signal_handler_Execution_can_be_stopped_by_SIGINT
  SOURCES "signal_handler_Execution_can_be_stopped_by_SIGINT.cpp"
  LIBRARIES fhg-util
)
fhg_add_test (NAME util_signal_handler_Execution_can_be_waited_multiple_times
  SOURCES "signal_handler_Execution_can_be_waited_multiple_times.cpp"
  LIBRARIES fhg-util
)
fhg_add_test (NAME util_signal_handler_Execution_can_be_stopped_multiple_times
  SOURCES "signal_handler_Execution_can_be_stopped_multiple_times.cpp"
  LIBRARIES fhg-util
)
