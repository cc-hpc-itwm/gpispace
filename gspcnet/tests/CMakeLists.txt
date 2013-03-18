# -*- mode: cmake; -*-

fhg_add_test(test_frame_parser.cpp
  LINK_LIBRARIES gspcnet
  DESCRIPTION "GSPCNet - frame parser tests"
)

fhg_add_test(test_queue_manager.cpp
  LINK_LIBRARIES gspcnet
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_THREAD_LIBS_INIT}
  DESCRIPTION "GSPCNet - queue manager tests"
)
