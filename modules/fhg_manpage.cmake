include(cmake_parse_arguments)
include(car_cdr_macros)

find_package (XmlTo)

macro(FHG_ADD_MANPAGE)
  PARSE_ARGUMENTS(MANP
    "OUTPUT;DESTINATION;COMPONENT;DEPENDS"
    "VERBOSE"
    ${ARGN}
    )

  if (XMLTO_FOUND)
    foreach (MANP_SOURCE ${MANP_DEFAULT_ARGS})

      string(REPLACE "." ";" _manp_components ${MANP_SOURCE})
      list(GET _manp_components 0 _manp_nam)
      list(GET _manp_components 1 _manp_cat)
      list(GET _manp_components 2 _manp_ext)

      set(_manp_out "${_manp_nam}.${_manp_cat}")

      if (NOT MANP_OUTPUT)
        set (MANP_OUTPUT "${_manp_out}")
      endif ()

      if (NOT MANP_DESTINATION)
        set (MANP_DESTINATION "share/man/man${_manp_cat}")
      endif()

      if (NOT MANP_COMPONENT)
        set (MANP_COMPONENT "sdk")
      endif()

      if (MANP_VERBOSE)
        message(STATUS "adding category ${_manp_cat} manpage ${MANP_OUTPUT} from source ${MANP_SOURCE}")
      endif()

      add_custom_command (OUTPUT "${MANP_OUTPUT}"
        COMMAND ${XMLTO_EXECUTABLE} man "${CMAKE_CURRENT_SOURCE_DIR}/${MANP_SOURCE}" 2>/dev/null
        DEPENDS "${MANP_SOURCE}")

      add_custom_target ("${MANP_OUTPUT}.manpage" ALL
        DEPENDS "${MANP_OUTPUT}")

      install (FILES "${CMAKE_CURRENT_BINARY_DIR}/${MANP_OUTPUT}" DESTINATION "${MANP_DESTINATION}" COMPONENT "${MANP_COMPONENT}")
    endforeach()
  else()
    message (WARNING "Cannot create manpage ${MANP_OUTPUT}, xmlto was not found.")
  endif()
endmacro()
