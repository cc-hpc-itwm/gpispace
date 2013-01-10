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
set (XML_LIBRARIES xml CACHE INTERNAL "xml_libraries" FORCE)

add_subdirectory (tests)
