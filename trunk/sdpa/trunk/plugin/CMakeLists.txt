include(fhg_plugin_macros)

# distributed runtime system plugin, aka interface to SDPA agent
fhg_add_plugin(drts drts.cpp master.cpp job.cpp
  EXPORTS DRTS
  LINK_LIBRARIES sdpa-events
                 sdpa-util
		 sdpa
		 ${FhgCom_LIBRARY}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS drts.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(kvs kvs.cpp
  EXPORTS KVS
  LINK_LIBRARIES ${FhgCom_LIBRARY}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS kvs.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(wfe wfe.cpp observable.cpp
  EXPORTS WFE
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${WE_LIBRARIES}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_FILESYSTEM_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS wfe.hpp observable.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(gui gui.cpp observable.cpp
  EXPORTS GUI
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${WE_LIBRARIES}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(sdpac sdpac.cpp
  EXPORTS SDPAC
  LINK_LIBRARIES sdpa-events
                 sdpa-util
		 sdpa
		 ${FhgCom_LIBRARY}
                 ${WE_PRECOMPILE_LIBRARIES}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS sdpac.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(net net.cpp
  EXPORTS Peer
  LINK_LIBRARIES ${FhgCom_LIBRARY}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS net.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)

fhg_add_plugin(sdpactl sdpactl.cpp
  EXPORTS Control
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS sdpactl.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
)
