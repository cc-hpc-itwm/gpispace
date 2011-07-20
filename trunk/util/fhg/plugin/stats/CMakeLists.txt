add_library(stats-plugin SHARED stats.cpp ../build.hpp)
set_target_properties(stats-plugin PROPERTIES OUTPUT_NAME stats)
