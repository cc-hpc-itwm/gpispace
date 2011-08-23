include(cmake_parse_arguments)
include(car_cdr_macros)

macro(FHG_ADD_PLUGIN)
  PARSE_ARGUMENTS(PLUGIN
    "EXPORTS;LINK_LIBRARIES"
    "BUILTIN;QUIET"
    ${ARGN}
    )
  CAR(PLUGIN_NAME ${PLUGIN_DEFAULT_ARGS})
  CDR(PLUGIN_SOURCES ${PLUGIN_DEFAULT_ARGS})

  if (QUIET)
  else()
    message(STATUS "**** Adding plugin ${PLUGIN_NAME} with sources ${PLUGIN_SOURCES}")
  endif()

  add_library(${PLUGIN_NAME}-plugin MODULE ${PLUGIN_SOURCES})
  set_target_properties(${PLUGIN_NAME}-plugin PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})
  set_target_properties(${PLUGIN_NAME}-plugin PROPERTIES PREFIX "")
  target_link_libraries(${PLUGIN_NAME}-plugin ${PLUGIN_LINK_LIBRARIES})

  string(TOUPPER ${PLUGIN_NAME} upper_name)
  if (PLUGIN_BUILTIN OR BUILTIN_${upper_name}_PLUGIN)
    add_library(${PLUGIN_NAME}-plugin.static ${PLUGIN_SOURCES})
    set_target_properties(${PLUGIN_NAME}-plugin.static PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})
    set_target_properties(${PLUGIN_NAME}-plugin.static PROPERTIES COMPILE_FLAGS "-DFHG_STATIC_PLUGIN=1")
    target_link_libraries(${PLUGIN_NAME}-plugin.static ${PLUGIN_LINK_LIBRARIES})
    list(APPEND builtin-plugins "${PLUGIN_NAME}-plugin.static")
  endif()
endmacro()
