fhg_add_test (test_config.cpp
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

fhg_add_test (test_handle_generator.cpp
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

fhg_add_test (test_memory_manager.cpp
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

fhg_add_test (test_pc_types.cpp
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

fhg_add_test (test_program_opts.cpp
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

fhg_add_test (test_segment.cpp
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

fhg_add_test (test_sfs_area.cpp dummy_topology.cpp
  PROJECT gpi-space
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 gpi-space-static
                 ${FhgCom_LIBRARY}
                 ${FhgLog_LIBRARY}
                 ${Boost_UNIT_TEST_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_LIBRARIES}
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (test_signal_handler.cpp
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

fhg_add_test (test_signal_handler_standalone.cpp
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

fhg_add_test (test_task.cpp
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
