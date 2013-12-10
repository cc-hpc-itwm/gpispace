find_package (LibReadline REQUIRED QUIET)
include_directories (SYSTEM ${LibReadline_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(tests
    activity.cpp
    execute_activity.cpp
    layer_atomics.cpp
    layer_cancel.cpp
    port.cpp
    property.cpp
    queue.cpp
    signal.cpp
    trans.cpp
    unordered_map.cpp
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

fhg_add_test (load_module.cpp
  PROJECT test_we
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
                 ${UTIL_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${LibReadline_LIBRARY}
                 ${CMAKE_DL_LIBS}
)

fhg_add_test (evaluate_net.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet ${FhgLog_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${CMAKE_DL_LIBS}
  DESCRIPTION "evaluate various nets and compare the results with given reference nets"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)

fhg_add_test (loader.cpp
  PROJECT we
  LINK_LIBRARIES pnet ${FhgLog_LIBRARY} ${CMAKE_DL_LIBS} ${Boost_FILESYSTEM_LIBRARY}
  DEPENDS question answer
)

add_library (answer SHARED answer.cpp)
add_library (question SHARED question.cpp)
target_link_libraries (question answer)
target_link_libraries (answer pnet)
