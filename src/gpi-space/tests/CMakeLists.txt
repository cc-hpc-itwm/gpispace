fhg_add_test (handle_generator.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
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
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
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
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (sfs_area.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 Boost::filesystem
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (task.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE SYSTEM "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)
