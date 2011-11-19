include(fhg_plugin_macros)

include(SDPAFindBoost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(FhgLog REQUIRED QUIET)
include_directories(${FhgLog_INCLUDE_DIR})

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

find_package(WE REQUIRED QUIET)
include_directories(${WE_INCLUDE_DIR})

# add include directory to sdpa-plugin directory
include_directories(${CMAKE_SOURCE_DIR}/sdpa/plugins)

# add include directory to gpi-plugin directory
include_directories(${CMAKE_SOURCE_DIR}/gpi-space)
include_directories(${CMAKE_SOURCE_DIR}/gpi-space/gpi-space/pc/plugin)

get_target_property(UFBMIG_INIT_INC pnet-ufbmig_init PNET_GENERATE_DIR)
include_directories(${UFBMIG_INIT_INC})

fhg_add_plugin(ufbmig_back ufbmig_back.cpp
  EXPORTS UFBMIG_BACK
  LINK_LIBRARIES ${WE_LIBRARIES}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS ufbmig_back.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
  DEPENDS pnet-ufbmig_init pnet-ufbmig_calc pnet-ufbmig_done pnet-ufbmig_mask
  INSTALL
)

fhg_add_plugin(ufbmig_front ufbmig_front.cpp
  EXPORTS UFBMIG_FRONT
  LINK_LIBRARIES ${PSPROMIGIF_LIBRARY}
                 ${HWLOC_SHARED_LIBRARY}
                 ${LIBIRC_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS ufbmig_front.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
  INSTALL
)

fhg_add_plugin(ufbmig_test ufbmig_test.cpp
  EXPORTS UFBMIG_TEST
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS ufbmig_front.hpp
  HEADER_DESTINATION include/plugins
  PLUGIN_DESTINATION libexec/plugins
  COMPONENT plugins
  INSTALL
)
