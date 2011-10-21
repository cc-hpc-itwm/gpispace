include(fhg_plugin_macros)

include(SDPAFindBoost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(FhgLog REQUIRED QUIET)
include_directories(${FhgLog_INCLUDE_DIR})

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

# add include directory to sdpa-plugin directory
include_directories(${CMAKE_SOURCE_DIR}/sdpa/plugin)

fhg_add_plugin(ufbmig_back ufbmig_back.cpp
  EXPORTS UFBMIG_BACK
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
)

fhg_add_plugin(ufbmig_front ufbmig_front.cpp
  EXPORTS UFBMIG_FRONT
  LINK_LIBRARIES ${PSPROMIGIF_LIBRARY}
                 ${HWLOC_SHARED_LIBRARY}
                 ${LIBIRC_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
		 ${Boost_THREAD_SHARED_LIBRARY}
		 ${Boost_SYSTEM_SHARED_LIBRARY}
)

install(FILES ufbmig_front.hpp ufbmig_back.hpp DESTINATION include/plugins COMPONENT headers)
install(TARGETS ufbmig_front-plugin ufbmig_back-plugin LIBRARY
  DESTINATION libexec/plugins
  COMPONENT plugins
  )
