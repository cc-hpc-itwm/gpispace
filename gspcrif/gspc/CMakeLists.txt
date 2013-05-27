set (gspcrif_src
  rif/util.cpp
  rif/pipe.cpp
  rif/proc_info.cpp
  rif/process.cpp
  rif/manager.cpp
  rif/convenience.cpp
)

add_library (gspcrif        STATIC ${gspcrif_src})
add_library (gspcrif.shared SHARED ${gspcrif_src})
set_target_properties (gspcrif.shared
  PROPERTIES OUTPUT_NAME gspcrif
             VERSION 0.1
)

install (TARGETS gspcrif ARCHIVE DESTINATION lib COMPONENT libraries)
install (TARGETS gspcrif.shared LIBRARY DESTINATION lib COMPONENT libraries)
