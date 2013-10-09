# -*- mode: cmake; -*-

fhg_add_test(test_parse_command_line.cpp
  LINK_LIBRARIES gspcrif
  DESCRIPTION "GSPCRif - command line parser tests"
)

fhg_add_test(test_process.cpp
  LINK_LIBRARIES gspcrif
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  DESCRIPTION "GSPCRif - process tests"
)

fhg_add_test(test_manager.cpp
  LINK_LIBRARIES gspcrif
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  DESCRIPTION "GSPCRif - process manager tests"
)

fhg_add_test(test_supervisor.cpp
  LINK_LIBRARIES gspcrif
                 fhg-util
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SIGNALS_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  DESCRIPTION "GSPCRif - process supervisor tests"
)
