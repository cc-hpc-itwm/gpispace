fhg_add_runtime_executable (NAME drts-kernel
  SOURCES "drts-kernel.cpp"
          "drts.cpp"
  LIBRARIES fhg-util
            fhgcom
            sdpa
            we-dev
            worker_module_loader
            rif-started_process_promise
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::system
            Boost::thread
            HWLOC::static
)

extended_add_library (NAME worker_module_loader
  SOURCES "context.cpp"
          "loader/Module.cpp"
          "loader/loader.cpp"
          "loader/module_call.cpp"
  TYPE STATIC POSITION_INDEPENDENT
  LIBRARIES Boost::filesystem
            Util::Generic
            fhglog
            gpi-space-pc-client-static
            gpi-space-pc-segment
            pnet
            ${CMAKE_DL_LIBS}
)

install (FILES "loader/wrapper.hpp"
  DESTINATION "include/module_call"
)
install (FILES "context_fwd.hpp"
               "context.hpp"
  DESTINATION include/drts/worker
)

fhg_add_test ("loader/test/loader.cpp"
  BOOST_UNIT_TEST
  PROJECT we_loader
  LINK_LIBRARIES worker_module_loader
                 order_stack
                 ${CMAKE_DL_LIBS}
  DEPENDS question
          answer
          order_a
          order_b
)

fhg_add_test ("loader/test/module.cpp"
  BOOST_UNIT_TEST
  PROJECT we_loader
  LINK_LIBRARIES worker_module_loader
                 fhglog
                 ${CMAKE_DL_LIBS}
  DEPENDS initialize_throws
          empty
)

fhg_add_test ("loader/test/module_requires_guard_symbol.cpp"
  BOOST_UNIT_TEST
  LINK_LIBRARIES worker_module_loader
  DEPENDS empty
)

extended_add_library (NAME answer TYPE SHARED SOURCES "loader/test/question_answer/answer.cpp")
extended_add_library (NAME question TYPE SHARED SOURCES "loader/test/question_answer/question.cpp" LIBRARIES answer)
extended_add_library (NAME order_stack TYPE SHARED SOURCES "loader/test/order/stack.cpp")
extended_add_library (NAME order_a TYPE SHARED SOURCES "loader/test/order/a.cpp" LIBRARIES order_stack)
extended_add_library (NAME order_b TYPE SHARED SOURCES "loader/test/order/b.cpp" LIBRARIES order_stack)
extended_add_library (NAME initialize_throws TYPE SHARED SOURCES "loader/test/module/initialize_throws.cpp")
extended_add_library (NAME empty TYPE SHARED SOURCES "loader/test/module/empty")
