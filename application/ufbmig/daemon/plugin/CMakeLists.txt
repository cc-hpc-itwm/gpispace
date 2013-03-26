include(fhg_plugin_macros)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(FhgLog REQUIRED QUIET)
include_directories(${FhgLog_INCLUDE_DIR})

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src)

# for usage of the sdpac plugin
find_package(SDPA REQUIRED QUIET)
include_directories(${SDPA_INCLUDE_DIR})

# plugin headers
find_package(FhgKernel REQUIRED QUIET)
include_directories(${FhgKernel_INCLUDE_DIR})

# add include directory to sdpa-plugin directory
include_directories(${CMAKE_SOURCE_DIR}/sdpa/plugins)

# add include directory to gpi-plugin directory
include_directories(${CMAKE_SOURCE_DIR}/gpi-space)
include_directories(${CMAKE_SOURCE_DIR}/gpi-space/gpi-space/pc/plugin)

get_target_property(UFBMIG_INIT_INC pnet-ufbmig_init PNET_GENERATE_DIR)
include_directories(${UFBMIG_INIT_INC})

set(PLUGIN_DESTINATION_DIR "libexec/fhg/plugins")
set(HEADER_DESTINATION_DIR "include/fhg/plugins")

fhg_add_plugin(ufbmig_back ufbmig_back.cpp
  EXPORTS UFBMIG_BACK
  LINK_LIBRARIES pnet
                 ${FhgLog_LIBRARY_SHARED}
                 ${Boost_FILESYSTEM_SHARED_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS ufbmig_back.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
  DEPENDS pnet-ufbmig_init pnet-ufbmig_calc pnet-ufbmig_done pnet-ufbmig_mask
  INSTALL
)

fhg_add_plugin(ufbmig_front ufbmig_front.cpp ${CMAKE_SOURCE_DIR}/sdpa/plugins/observable.cpp
  EXPORTS UFBMIG_FRONT
  LINK_LIBRARIES ${PSPROMIGIF_LIBRARY}
                 ${FhgLog_LIBRARY_SHARED}
                 ${HWLOC_SHARED_LIBRARY}
                 ${LIBIRC_LIBRARY}
                 ${LOG4CPP_LIBRARY}
                 ${Boost_FILESYSTEM_SHARED_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS ufbmig_front.hpp ufbmig_msg_types.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
  INSTALL
)

fhg_add_plugin(ufbmig_test ufbmig_test.cpp
  EXPORTS UFBMIG_TEST
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS ufbmig_front.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
  INSTALL
)

fhg_add_plugin(isim_dummy isim_dummy.cpp
  EXPORTS ISIM
  LINK_LIBRARIES ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_FILESYSTEM_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS isim.hpp reactor.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
  INSTALL
)

fhg_add_plugin(isim isim.cpp
  EXPORTS ISIM
  LINK_LIBRARIES ${PSPROMIGIF_LIBRARY}
                 ${FhgLog_LIBRARY_SHARED}
                 ${HWLOC_SHARED_LIBRARY}
                 ${LIBIRC_LIBRARY}
                 ${LOG4CPP_LIBRARY}
                 ${Boost_SERIALIZATION_SHARED_LIBRARY}
                 ${Boost_FILESYSTEM_SHARED_LIBRARY}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  HEADERS isim.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
  INSTALL
)
