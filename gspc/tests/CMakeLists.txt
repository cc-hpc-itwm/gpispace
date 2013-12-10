# -*- mode: cmake; -*-

fhg_add_test(test_frame_parser.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - frame parser tests"
)

fhg_add_test(test_util.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - util tests"
)

fhg_add_test(test_frame_builder.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - frame builder tests"
)

fhg_add_test(test_queue_manager.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - queue manager tests"
)

fhg_add_test(test_demux.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - demux tests"
)

fhg_add_test(test_services.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - services tests"
)

fhg_add_test(test_serve.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - serve tests"
)

fhg_add_test(test_auth.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - auth tests"
)

fhg_add_test(test_heartbeat_info.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - heartbeat info tests"
)

fhg_add_test(test_io.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - test the global io"
)

fhg_add_test(test_parse_command_line.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc ${Boost_FILESYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - command line parser tests"
)

fhg_add_test(test_process.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - process tests"
)

fhg_add_test(test_manager.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - process manager tests"
)

fhg_add_test(test_supervisor.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 fhg-util
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - process supervisor tests"
)

#### KVS related tests

fhg_add_test(test_kvs_impl.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 fhg-util
                 we-dev
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_DL_LIBS}
  DESCRIPTION "GSPCKvs - KVS implementation tests"
)

fhg_add_test(test_kvs_net.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES libgspc
                 fhg-util
                 we-dev
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_DL_LIBS}
  DESCRIPTION "GSPCKvs - KVS network tests"
)
