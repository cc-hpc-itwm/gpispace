# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (loader.cpp
  BOOST_UNIT_TEST
  PROJECT we_loader
  LINK_LIBRARIES pnet
                 order_stack
                 ${FhgLog_LIBRARY}
                 ${CMAKE_DL_LIBS}
                 ${Boost_FILESYSTEM_LIBRARY}
  DEPENDS question
          answer
          order_a
          order_b
)

add_library (answer SHARED question_answer/answer.cpp)
add_library (question SHARED question_answer/question.cpp)
target_link_libraries (question answer)
target_link_libraries (answer pnet)

add_library (order_stack SHARED order/stack.cpp)
add_library (order_a SHARED order/a.cpp)
add_library (order_b SHARED order/b.cpp)
target_link_libraries (order_a order_stack pnet)
target_link_libraries (order_b order_stack pnet)
