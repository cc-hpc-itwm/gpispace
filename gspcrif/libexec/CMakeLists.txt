include(fhg_plugin_macros)

find_package (FhgKernel REQUIRED QUIET)
include_directories (${FhgKernel_INCLUDE_DIR})

find_package (FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

find_package(GSPC_NET REQUIRED QUIET)
include_directories(${GSPC_NET_INCLUDE_DIR})

set(PLUGIN_DESTINATION_DIR "libexec/fhg/plugins")
set(HEADER_DESTINATION_DIR "include/fhg/plugins")

fhg_add_plugin(rif rif.cpp
  EXPORTS RIF
  LINK_LIBRARIES gspcrif.shared
                 ${GSPC_NET_LIBRARY_SHARED}
                 ${FhgLog_LIBRARY_SHARED}
                 ${Boost_THREAD_SHARED_LIBRARY}
                 ${Boost_DATE_TIME_SHARED_LIBRARY}
                 ${Boost_SYSTEM_SHARED_LIBRARY}
  INSTALL
  HEADERS rif.hpp
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)
