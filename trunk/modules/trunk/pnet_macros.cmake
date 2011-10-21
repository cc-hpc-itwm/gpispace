include(cmake_parse_arguments)
include(car_cdr_macros)

macro(PNET_COMPILE)
  PARSE_ARGUMENTS(PNET
    "INCLUDES;GENERATE;OUTPUT;FLAGS;INSTALL;DEPENDS"
    "QUIET;BUILD"
    ${ARGN}
    )
  CAR(PNET_NAME ${PNET_DEFAULT_ARGS})
  CDR(PNET_SOURCES ${PNET_DEFAULT_ARGS})

  set(PNETC_LOCATION pnetc)
  if (TARGET pnetc)
    get_target_property(PNETC_LOCATION pnetc LOCATION)
  endif()

  set (PNET__default_flags "--Woverwrite-file=false")

  if (PNET_QUIET)
  else()
    message(STATUS "**** Adding pnet ${PNET_NAME} with source ${PNET_SOURCES}")
  endif()

  if (PNET_INCLUDES)
    foreach(p ${PNET_INCLUDES})
      if (IS_ABSOLUTE ${p})
      else()
	set(p ${CMAKE_CURRENT_SOURCE_DIR}/${p})
      endif()
      set (PNET_FLAGS ${PNET_FLAGS} -I ${p})
    endforeach()
  endif()

  if (PNET_OUTPUT)
  else()
    set(PNET_OUTPUT ${PNET_NAME}.pnet)
  endif()

  set(__pnet_sources)
  if (PNET_SOURCES)
    foreach (s ${PNET_SOURCES})
      if (IS_ABSOLUTE ${s})
	set(__pnet_sources ${__pnet_sources} ${s})
      else()
	set(__pnet_sources ${__pnet_sources} ${CMAKE_CURRENT_SOURCE_DIR}/${s})
      endif()
    endforeach()
  else()
    message(FATAL_ERROR "** pnet_compile: at least one source file is required")
  endif()

  set(PNETC_ARGS ${PNET__default_flags} ${PNET_FLAGS} ${__pnet_sources} -o ${PNET_OUTPUT})

  if (PNET_GENERATE)
    set(PNETC_ARGS ${PNETC_ARGS} -g ${PNET_GENERATE})
  endif()

  add_custom_target(pnet-${PNET_NAME} ALL
    COMMAND ${PNETC_LOCATION} ${PNETC_ARGS}
    DEPENDS ${PNETC_LOCATION} ${PNET_DEPENDS}
    COMMENT "Compiling petri-net: ${PNET_NAME}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES ${PNET_SOURCES}
    )

  set_target_properties(pnet-${PNET_NAME} PROPERTIES PNET_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PNET_GENERATE})

  if (PNET_BUILD)
    add_custom_command(TARGET pnet-${PNET_NAME} POST_BUILD
      COMMAND make -C ${PNET_GENERATE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
  endif()

  if (TARGET pnetc)
    add_dependencies(pnet-${PNET_NAME} pnetc)
  endif()

  if (PNET_INSTALL)
    install (FILES ${__pnet_sources} ${PNET_OUTPUT} DESTINATION ${PNET_INSTALL})
  endif()
endmacro()
