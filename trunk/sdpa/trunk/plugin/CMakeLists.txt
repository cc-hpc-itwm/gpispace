include(fhg_plugin_macros)

#
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#

# distributed runtime system plugin, aka interface to SDPA agent
fhg_add_plugin(drts drts.cpp master.cpp job.cpp
  EXPORTS DRTS
  LINK_LIBRARIES sdpa-events
                 sdpa-util
		 sdpa
		 ${FhgCom_LIBRARY}
		 ${BOOST_ROOT}/lib/libboost_serialization.so
)

install(FILES drts.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS drts-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )

fhg_add_plugin(kvs kvs.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${BOOST_ROOT}/lib/libboost_serialization.so
)

install(FILES kvs.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS kvs-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )

fhg_add_plugin(wfe wfe.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${WE_PRECOMPILE_LIBRARIES}
                 ${BOOST_ROOT}/lib/libboost_serialization.so
)

install(FILES wfe.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS wfe-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )
