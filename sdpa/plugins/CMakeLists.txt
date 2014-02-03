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

fhg_add_plugin(net net.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS net.hpp
)
