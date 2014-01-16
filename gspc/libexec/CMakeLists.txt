include(fhg_plugin_macros)

include_directories (${FhgKernel_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

set(PLUGIN_DESTINATION_DIR "libexec/fhg/plugins")
set(HEADER_DESTINATION_DIR "include/fhg/plugins")

add_library (rifimpl rif.cpp)
target_link_libraries (rifimpl libgspc
                 ${FhgLog_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_plugin(kvsd2 kvsd.cpp
  EXPORTS KVSD
  LINK_LIBRARIES libgspc.shared
                 ${FhgLog_LIBRARY_SHARED}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SIGNALS_LIBRARY}
                 ${Boost_REGEX_LIBRARY}
                 ${Boost_DATE_TIME_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)
