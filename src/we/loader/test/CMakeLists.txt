# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (loader.cpp
  BOOST_UNIT_TEST
  PROJECT we_loader
  LINK_LIBRARIES pnet
                 order_stack
                 ${CMAKE_DL_LIBS}
  DEPENDS question
          answer
          order_a
          order_b
)

fhg_add_test (module.cpp
  BOOST_UNIT_TEST
  PROJECT we_loader
  LINK_LIBRARIES pnet
                 fhglog
                 ${CMAKE_DL_LIBS}
  DEPENDS initialize_throws
          empty_not_linked_with_pnet
          empty
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

add_library (initialize_throws SHARED module/initialize_throws.cpp)
target_link_libraries (initialize_throws pnet)

add_library (empty_not_linked_with_pnet
  SHARED module/empty_not_linked_with_pnet.cpp
)

add_library (empty SHARED module/empty)
target_link_libraries (empty pnet)
