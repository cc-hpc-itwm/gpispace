include(fhg_plugin_macros)

set(PLUGIN_DESTINATION_DIR "libexec/fhg/plugins")
set(HEADER_DESTINATION_DIR "include/fhg/plugins")

# distributed runtime system plugin, aka interface to SDPA agent
fhg_add_plugin(drts drts.cpp master.cpp job.cpp drts_callbacks.cpp
  EXPORTS DRTS
  LINK_LIBRARIES sdpa-events
                 sdpa-util
                 sdpa
                 ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS drts.hpp drts_callbacks.h
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(kvsd kvsd.cpp
  EXPORTS KVSD
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${FhgLog_LIBRARY_SHARED}
                 fhg-util
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(kvs kvs.cpp
  EXPORTS KVS
  LINK_LIBRARIES fhg-util
                 ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS kvs.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(kvs_dummy kvs_dummy.cpp
  EXPORTS KVS
  LINK_LIBRARIES ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS kvs.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(wfe wfe.cpp observable.cpp
  EXPORTS WFE
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 pnet
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_FILESYSTEM_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS wfe.hpp observable.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(wfe_eval wfe_eval.cpp
  EXPORTS WFE_EVAL
  LINK_LIBRARIES pnet
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_FILESYSTEM_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(gui gui.cpp observable.cpp
  EXPORTS GUI
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 pnet
                 fhg-util
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(sdpac sdpac.cpp
  EXPORTS SDPAC
  LINK_LIBRARIES sdpa-events
                 sdpa-util
                 sdpa
                 ${FhgCom_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS sdpac.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
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
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(sdpactl sdpactl.cpp
  EXPORTS Control
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS sdpactl.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(progress progress.cpp
  EXPORTS Progress
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS progress.hpp progress.h
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(logd logd.cpp observable.cpp
  EXPORTS Logd
  LINK_LIBRARIES ${FhgLog_LIBRARY_SHARED}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)

fhg_add_plugin(logc logc.cpp
  EXPORTS Logc
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS logc.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)
