set (tests
  test_config.cpp
  test_signal_handler.cpp
  test_signal_handler_standalone.cpp
  test_pc_types.cpp
  test_segment.cpp
  test_handle_generator.cpp
  test_memory_manager.cpp
  test_task.cpp
  test_sfs_area.cpp
  test_program_opts.cpp
)

foreach (test ${tests})
  fhg_add_test (${test} dummy_topology.cpp
    PROJECT gpi-space
    LINK_LIBRARIES gpi-space-pc-segment
                   gpi-space-pc-memory
                   gpi-space-static
                   ${FhgCom_LIBRARY}
                   ${FhgLog_LIBRARY}
                   ${Boost_UNIT_TEST_LIBRARIES}
                   ${Boost_LIBRARIES}
                   ${CMAKE_THREAD_LIBS_INIT}
                   fhg-util
  )
endforeach()
