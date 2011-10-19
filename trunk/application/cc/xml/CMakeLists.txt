configure_file(compile.xml.in ${CMAKE_CURRENT_BINARY_DIR}/compile.xml @ONLY)

# TODO:
#    find pnetc (installation or within build-tree)
#        see: target_property_path
#    write macro to generate the code + building it
#    let pnetc generate cmake files...
get_target_property(PNETC_LOCATION pnetc LOCATION)
if (PNETC_LOCATION)
else()
  set(PNETC_LOCATION pnetc)
endif()
add_custom_target(gen-sdpacc ALL
  COMMAND ${PNETC_LOCATION} --Woverwrite-file=false ${CMAKE_CURRENT_BINARY_DIR}/compile.xml -g ${CMAKE_CURRENT_BINARY_DIR}/gen -o ${CMAKE_CURRENT_BINARY_DIR}/compile.pnet
  DEPENDS ${PNETC_LOCATION}
  COMMENT "generating sdpacc module"
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/compile.xml
  )
add_dependencies(gen-sdpacc pnetc)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/compile.pnet PROPERTIES GENERATED true)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/gen PROPERTIES GENERATED true)

#add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/gen)

install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/compile.xml
  ${CMAKE_CURRENT_BINARY_DIR}/compile.pnet
  DESTINATION share/sdpa/cc
)
