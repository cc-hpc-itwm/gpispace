configure_file(compile.xml.in compile.xml @ONLY)

# TODO:
#    find pnetc (installation or within build-tree)
#        see: target_property_path
#    write macro to generate the code + building it
#    let pnetc generate cmake files...
add_custom_command(COMMAND pnetc ${CMAKE_CURRENT_BUILD_DIR}/compile.xml -g ${CMAKE_CURRENT_BUILD_DIR}/gen -o compile.pnet
  OUTPUT compile.pnet
  DEPENDS pnetc
  COMMENT "generating sdpacc module"
  )
set_source_files_properties(compile.pnet PROPERTIES GENERATED true)

install (FILES
  compile.xml
  compile.pnet
  DESTINATION share/sdpa/cc
)
