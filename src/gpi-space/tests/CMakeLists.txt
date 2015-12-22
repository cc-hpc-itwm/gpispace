fhg_add_test (handle_generator.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (memory_area.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (segment.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (beegfs_area.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  ARGS --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 Boost::filesystem
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
                 test-utilities
)
