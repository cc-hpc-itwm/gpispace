fhg_add_test (NAME we_loader_loader
  SOURCES loader.cpp
  USE_BOOST
  LIBRARIES pnet
            order_stack
            ${CMAKE_DL_LIBS}
  DEPENDS question
          answer
          order_a
          order_b
)

fhg_add_test (NAME we_loader_module
  SOURCES module.cpp
  USE_BOOST
  LIBRARIES pnet
            ${CMAKE_DL_LIBS}
  DEPENDS initialize_throws
          empty_not_linked_with_pnet
          empty
)

fhg_add_test (NAME we_loader_module-needs_api_guard
  SOURCES "module-needs_api_guard.cpp"
          # Can't link: not part of public API, and linking would add
          # GPISpace::APIGuard to linked libraries, defeating the
          # purpose of the test.
          "../Module.cpp"
          "../exceptions.cpp"
  USE_BOOST
  LIBRARIES GPISpace::header-only
            Util::Generic
            PRIVATE fmt
  DEPENDS empty_not_linked_with_pnet
)

gspc_add_library (NAME answer
  TYPE SHARED
  SOURCES "question_answer/answer.cpp"
  LIBRARIES pnet
            Util::Generic
)
gspc_add_library (NAME question
  TYPE SHARED
  SOURCES "question_answer/question.cpp"
  LIBRARIES answer
            Util::Generic
)

gspc_add_library (NAME order_stack
  TYPE SHARED
  SOURCES "order/stack.cpp"
  LIBRARIES Util::Generic
)
gspc_add_library (NAME order_a
  TYPE SHARED
  SOURCES "order/a.cpp"
  LIBRARIES order_stack
            pnet
            Util::Generic
)
gspc_add_library (NAME order_b
  TYPE SHARED
  SOURCES "order/b.cpp"
  LIBRARIES order_stack
            pnet
            Util::Generic
)

gspc_add_library (NAME initialize_throws
  TYPE SHARED
  SOURCES "module/initialize_throws.cpp"
  LIBRARIES GPISpace::header-only
            pnet
)
gspc_add_library (NAME empty_not_linked_with_pnet
  TYPE SHARED
  SOURCES "module/empty_not_linked_with_pnet.cpp"
  LIBRARIES GPISpace::header-only
            Boost::base
)
gspc_add_library (NAME empty
  TYPE SHARED
  SOURCES "module/empty.cpp"
  LIBRARIES GPISpace::header-only
            pnet
)
gspc_add_library (NAME empty_nodelete
  TYPE SHARED
  SOURCES "module/empty.cpp"
  LIBRARIES GPISpace::header-only
            pnet
)
# Explicitly mark the library as not being unloaded if `dlclose()` is
# called on it. This emulates the behavior that some libraries with
# global variables have (see `STB_GNU_UNIQUE`,`-fno-gnu-unique`).
target_link_options (empty_nodelete PUBLIC LINKER:-z,nodelete)
