# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (loader.cpp
  PROJECT test_we_loader
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
                 ${CMAKE_DL_LIBS}
                 ${Boost_FILESYSTEM_LIBRARY}
  DEPENDS question
          answer
)

fhg_add_test (load_module.cpp
  PROJECT test_we_loader
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY}
                 ${UTIL_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${LibReadline_LIBRARY}
                 ${CMAKE_DL_LIBS}
)

add_library (answer SHARED answer.cpp)
add_library (question SHARED question.cpp)
target_link_libraries (question answer)
target_link_libraries (answer pnet)
