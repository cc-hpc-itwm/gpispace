include(fhg_plugin_macros)

#set (BUILTIN_STATS_PLUGIN ON)

fhg_add_plugin(stats stats.cpp)
fhg_add_plugin(hello hello.cpp)
fhg_add_plugin(world world.cpp)
fhg_add_plugin(helloworld helloworld.cpp)

set(PLUGIN_DESTINATION_DIR "libexec/fhg/plugins")
set(HEADER_DESTINATION_DIR "include/fhg/plugins")

fhg_add_plugin(numa numa.cpp
  EXPORTS NUMA
  LINK_LIBRARIES ${FhgLog_LIBRARY_SHARED}
                 ${HWLOC_SHARED_LIBRARY}
  INSTALL
  HEADER_DESTINATION "${HEADER_DESTINATION_DIR}"
  PLUGIN_DESTINATION "${PLUGIN_DESTINATION_DIR}"
  COMPONENT plugins
)
