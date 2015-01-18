fhg_add_test (test_handle_generator.cpp
  PROJECT gpi-space
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 ${FhgLog_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (test_memory_area.cpp
  PROJECT gpi-space
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 ${FhgLog_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (test_segment.cpp
  PROJECT gpi-space
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 ${FhgLog_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (test_sfs_area.cpp
  PROJECT gpi-space
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (test_task.cpp
  PROJECT gpi-space
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 ${FhgLog_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)
