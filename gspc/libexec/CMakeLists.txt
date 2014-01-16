include(fhg_plugin_macros)

include_directories (${FhgKernel_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

fhg_add_plugin(kvsd2 kvsd.cpp
  LINK_LIBRARIES libgspc.shared
                 ${FhgLog_LIBRARY_SHARED}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_REGEX_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
)
