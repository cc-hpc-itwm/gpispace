find_package (LibReadline REQUIRED QUIET)
include_directories (SYSTEM ${LibReadline_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(tests
    execute_activity.cpp
    layer_atomics.cpp
    layer_cancel.cpp
    port.cpp
    property.cpp
    trans.cpp
   )

foreach (test ${tests})
  fhg_add_test (${test}
    PROJECT we
    BOOST_UNIT_TEST # all of them?
    LINK_LIBRARIES pnet
                   ${FhgLog_LIBRARY}
                   ${UTIL_LIBRARIES}
                   ${Boost_THREAD_LIBRARY} # layer_atomics, layer_cancel, queue
                   ${LibReadline_LIBRARY}
                   ${CMAKE_DL_LIBS}
  )
endforeach()

fhg_add_test (evaluate_net.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet ${FhgLog_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${CMAKE_DL_LIBS}
  DESCRIPTION "evaluate various nets and compare the results with given reference nets"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)
