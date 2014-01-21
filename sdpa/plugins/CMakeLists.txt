include(fhg_plugin_macros)

fhg_add_plugin(kvsd kvsd.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${FhgLog_LIBRARY_SHARED}
                 fhg-util
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
)

fhg_add_plugin(kvs kvs.cpp
  LINK_LIBRARIES fhg-util
                 ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS kvs.hpp
)

fhg_add_plugin(kvs_dummy kvs_dummy.cpp
  LINK_LIBRARIES ${Boost_THREAD_LIBRARY}
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

fhg_add_plugin(progress progress.cpp
  LINK_LIBRARIES ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS progress.hpp progress.h
)

fhg_add_plugin(logd logd.cpp observable.cpp
  LINK_LIBRARIES ${FhgLog_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
)

fhg_add_plugin(netd netd.cpp
  LINK_LIBRARIES ${GSPC_LIBRARY_SHARED}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
)
