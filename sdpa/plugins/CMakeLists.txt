include(fhg_plugin_macros)

fhg_add_plugin(kvs kvs.cpp
  LINK_LIBRARIES fhg-util
                 ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS kvs.hpp
)

fhg_add_plugin(sdpac sdpac.cpp
  LINK_LIBRARIES sdpa
                 ${FhgCom_LIBRARY}
                 ${GSPC_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS sdpac.hpp
)

fhg_add_plugin(net net.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS net.hpp
)

fhg_add_plugin(sdpactl sdpactl.cpp
  LINK_LIBRARIES ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS sdpactl.hpp
)

fhg_add_plugin(logd logd.cpp observable.cpp
  LINK_LIBRARIES ${FhgLog_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
)
