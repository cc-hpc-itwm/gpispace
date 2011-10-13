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
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
)

install(FILES drts.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS drts-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )

fhg_add_plugin(kvs kvs.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
)

install(FILES kvs.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS kvs-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )

fhg_add_plugin(wfe wfe.cpp observable.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${WE_LIBRARIES}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_FILESYSTEM_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
)

install(FILES wfe.hpp observable.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS wfe-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )

fhg_add_plugin(gui gui.cpp
  LINK_LIBRARIES ${FhgCom_LIBRARY}
                 ${WE_LIBRARIES}
		 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
)

#install(FILES gui.hpp observer.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS gui-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )
