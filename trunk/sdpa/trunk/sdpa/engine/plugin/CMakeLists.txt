include(fhg_plugin_macros)

#
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#

fhg_add_plugin(wfe_test wfe_test.cpp 
  LINK_LIBRARIES ${FhgCom_LIBRARY}
		         ${Boost_SERIALIZATION_SHARED_LIBRARY}
		         ${Boost_FILESYSTEM_SHARED_LIBRARY}
		         ${Boost_THREAD_SHARED_LIBRARY}
		         ${Boost_SYSTEM_SHARED_LIBRARY}
)