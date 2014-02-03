include(fhg_plugin_macros)

fhg_add_plugin(logd logd.cpp observable.cpp
  LINK_LIBRARIES ${FhgLog_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
)
