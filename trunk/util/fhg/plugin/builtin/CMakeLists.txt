add_library(simple-stats-plugin MODULE stats.cpp)
set_target_properties(simple-stats-plugin PROPERTIES OUTPUT_NAME simple_stats)
set_target_properties(simple-stats-plugin PROPERTIES PREFIX "")

if(BUILTIN_STATS_PLUGIN)
  add_library(simple-stats-plugin.static stats.cpp)
  set_target_properties(simple-stats-plugin.static PROPERTIES OUTPUT_NAME simple_stats)
  set_target_properties(simple-stats-plugin.static PROPERTIES COMPILE_FLAGS "-DFHG_STATIC_PLUGIN=1")
  list(APPEND builtin-plugins "simple-stats-plugin.static")
endif()

include(libutils)
merge_static_libs(fhg-builtin-plugins fhg-builtin-plugins "${builtin-plugins}")
