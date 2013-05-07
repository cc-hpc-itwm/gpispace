include(cmake_parse_arguments)
include(car_cdr_macros)

macro(FHG_ADD_PLUGIN)
  PARSE_ARGUMENTS(PLUGIN
    "EXPORTS;LINK_LIBRARIES;HEADERS;HEADER_DESTINATION;PLUGIN_DESTINATION;COMPONENT;DEPENDS"
    "BUILTIN;VERBOSE;INSTALL"
    ${ARGN}
    )
  CAR(PLUGIN_NAME ${PLUGIN_DEFAULT_ARGS})
  CDR(PLUGIN_SOURCES ${PLUGIN_DEFAULT_ARGS})

  if (PLUGIN_VERBOSE)
    message(STATUS "adding plugin ${PLUGIN_NAME} with sources ${PLUGIN_SOURCES}")
  endif()

  add_library(${PLUGIN_NAME}-plugin MODULE ${PLUGIN_SOURCES})
  set_target_properties(${PLUGIN_NAME}-plugin PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})
  set_target_properties(${PLUGIN_NAME}-plugin PROPERTIES PREFIX "")
  target_link_libraries(${PLUGIN_NAME}-plugin ${PLUGIN_LINK_LIBRARIES} ${Boost_SERIALIZATION_SHARED_LIBRARY})
  foreach (d ${PLUGIN_DEPENDS})
    add_dependencies(${PLUGIN_NAME}-plugin ${d})
  endforeach()

  if(PLUGIN_INSTALL)
    install(TARGETS ${PLUGIN_NAME}-plugin LIBRARY DESTINATION ${PLUGIN_PLUGIN_DESTINATION} COMPONENT ${PLUGIN_COMPONENT})
    if (PLUGIN_HEADERS)
      install(FILES ${PLUGIN_HEADERS} DESTINATION ${PLUGIN_HEADER_DESTINATION} COMPONENT ${PLUGIN_COMPONENT})
    endif()
  endif()

  string(TOUPPER ${PLUGIN_NAME} upper_name)
  if (PLUGIN_BUILTIN OR BUILTIN_${upper_name}_PLUGIN)
    add_library(${PLUGIN_NAME}-plugin.static ${PLUGIN_SOURCES})
    set_target_properties(${PLUGIN_NAME}-plugin.static PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})
    set_target_properties(${PLUGIN_NAME}-plugin.static PROPERTIES COMPILE_FLAGS "-DFHG_STATIC_PLUGIN=1")
    target_link_libraries(${PLUGIN_NAME}-plugin.static ${PLUGIN_LINK_LIBRARIES})
    foreach (d ${PLUGIN_DEPENDS})
      add_dependencies(${PLUGIN_NAME}-plugin.static ${d})
    endforeach()
    list(APPEND builtin-plugins "${PLUGIN_NAME}-plugin.static")
    if (PLUGIN_INSTALL)
      install(TARGETS ${PLUGIN_NAME}-plugin.static ARCHIVE DESTINATION ${PLUGIN_PLUGIN_DESTINATION} COMPONENT ${PLUGIN_COMPONENT})
    endif()
  endif()
endmacro()
