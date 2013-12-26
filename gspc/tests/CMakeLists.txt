fhg_add_test(frame_parser.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - frame parser tests"
)

fhg_add_test(util.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - util tests"
)

fhg_add_test(frame_builder.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
  DESCRIPTION "GSPCNet - frame builder tests"
)

fhg_add_test(queue_manager.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - queue manager tests"
)

fhg_add_test(demux.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - demux tests"
)

fhg_add_test(services.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - services tests"
)

fhg_add_test(serve.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
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

fhg_add_test(auth.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_FILE_SYSTEM_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - auth tests"
)

fhg_add_test(io.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
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

fhg_add_test(parse_command_line.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc ${Boost_FILESYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - command line parser tests"
)

fhg_add_test(process.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - process tests"
)

fhg_add_test(manager.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
  LINK_LIBRARIES libgspc
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  DESCRIPTION "GSPCRif - process manager tests"
)

fhg_add_test(supervisor.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
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

fhg_add_test(kvs_impl.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
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

fhg_add_test(kvs_net.cpp
  BOOST_UNIT_TEST
  PROJECT gspc
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
