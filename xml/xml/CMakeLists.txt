function (add_xml_sources)
  foreach (source ${ARGN})
    set (XML_SOURCES ${XML_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/${source}
      CACHE INTERNAL "xml_sources" FORCE
    )
  endforeach()
endfunction()

add_subdirectory (parse)

add_library (xml ${XML_SOURCES})
target_link_libraries (xml
  fhg-util
  ${Boost_LIBRARIES}
)

get_target_property(GEN_INCLUDE_EXE gen-includes LOCATION)

set (headerlist_hpp ${CMAKE_CURRENT_BINARY_DIR}/parse/headerlist.hpp)

add_custom_command(OUTPUT ${headerlist_hpp}
  COMMAND ${GEN_INCLUDE_EXE} -o ${headerlist_hpp}
  DEPENDS gen-includes ${CMAKE_CURRENT_BINARY_DIR}/parse/includes.hpp
  COMMENT "generating internal pnetc headerlist"
)

set_source_files_properties (
  "${CMAKE_CURRENT_SOURCE_DIR}/parse/parser.cpp"
  PROPERTIES OBJECT_DEPENDS "${headerlist_hpp}"
)

add_subdirectory (tests)
