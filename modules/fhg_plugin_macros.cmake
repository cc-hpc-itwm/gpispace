include(cmake_parse_arguments)
include(car_cdr_macros)

macro(FHG_ADD_PLUGIN)
  PARSE_ARGUMENTS(PLUGIN
    "LINK_LIBRARIES;HEADERS;DEPENDS"
    "VERBOSE;INSTALL"
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
  set_target_properties(${PLUGIN_NAME}-plugin PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_RPATH}")
  target_link_libraries(${PLUGIN_NAME}-plugin ${PLUGIN_LINK_LIBRARIES} ${Boost_SERIALIZATION_SHARED_LIBRARY})
  foreach (d ${PLUGIN_DEPENDS})
    add_dependencies(${PLUGIN_NAME}-plugin ${d})
  endforeach()

  set(PLUGIN_DESTINATION_DIR "libexec/fhg/plugins")
  set(HEADER_DESTINATION_DIR "include/fhg/plugins")

  if(PLUGIN_INSTALL)
    install(TARGETS ${PLUGIN_NAME}-plugin LIBRARY DESTINATION ${PLUGIN_DESTINATION_DIR} COMPONENT "plugins")
    if (PLUGIN_HEADERS)
      install(FILES ${PLUGIN_HEADERS} DESTINATION ${HEADER_DESTINATION_DIR} COMPONENT "plugins")
    endif()
  endif()
endmacro()
