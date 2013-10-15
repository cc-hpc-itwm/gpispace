# -*- mode: cmake; -*-

fhg_add_test(test_frame_parser.cpp
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - frame parser tests"
)

fhg_add_test(test_util.cpp
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - util tests"
)

fhg_add_test(test_frame_builder.cpp
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - frame builder tests"
)

fhg_add_test(test_queue_manager.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - queue manager tests"
)

fhg_add_test(test_demux.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - demux tests"
)

fhg_add_test(test_services.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - services tests"
)

fhg_add_test(test_serve.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - serve tests"
)

fhg_add_test(test_auth.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - auth tests"
)

fhg_add_test(test_io.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - test the global io"
)

fhg_add_test(test_parse_command_line.cpp
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCRif - command line parser tests"
)

fhg_add_test(test_process.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  DESCRIPTION "GSPCRif - process tests"
)

fhg_add_test(test_manager.cpp
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  DESCRIPTION "GSPCRif - process manager tests"
)

fhg_add_test(test_supervisor.cpp
  LINK_LIBRARIES libgspc
                 fhg-util
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SIGNALS_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  DESCRIPTION "GSPCRif - process supervisor tests"
)

#### KVS related tests

fhg_add_test(test_kvs.cpp
  LINK_LIBRARIES libgspc
                 fhg-util
                 we-dev
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SIGNALS_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
                 ${CMAKE_DL_LIBS}
  DESCRIPTION "GSPCRif - process supervisor tests"
)
