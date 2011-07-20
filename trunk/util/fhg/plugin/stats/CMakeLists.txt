add_library(stats-plugin SHARED stats.cpp)
add_library(stats-plugin.static stats.cpp)
set_target_properties(stats-plugin PROPERTIES OUTPUT_NAME stats)
set_target_properties(stats-plugin.static PROPERTIES OUTPUT_NAME stats)
