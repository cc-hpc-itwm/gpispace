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

fhg_add_test (module_requires_guard_symbol.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
)

extended_add_library (NAME answer TYPE SHARED SOURCES "question_answer/answer.cpp" LIBRARIES pnet)
extended_add_library (NAME question TYPE SHARED SOURCES "question_answer/question.cpp" LIBRARIES answer)
extended_add_library (NAME order_stack TYPE SHARED SOURCES "order/stack.cpp")
extended_add_library (NAME order_a TYPE SHARED SOURCES "order/a.cpp" LIBRARIES order_stack pnet)
extended_add_library (NAME order_b TYPE SHARED SOURCES "order/b.cpp" LIBRARIES order_stack pnet)
extended_add_library (NAME initialize_throws TYPE SHARED SOURCES "module/initialize_throws.cpp" LIBRARIES pnet)
extended_add_library (NAME empty_not_linked_with_pnet TYPE SHARED SOURCES "module/empty_not_linked_with_pnet.cpp")
extended_add_library (NAME empty TYPE SHARED SOURCES "module/empty" LIBRARIES pnet)
