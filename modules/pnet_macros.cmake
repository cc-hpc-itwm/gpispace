include(cmake_parse_arguments)
include(car_cdr_macros)

macro(PNET_COMPILE)
  PARSE_ARGUMENTS(PNET
    "INCLUDES;GENERATE;OUTPUT;FLAGS;INSTALL;DEPENDS;LDFLAGS;CXXFLAGS;COMPONENT"
    "QUIET;BUILD"
    ${ARGN}
    )
  CAR(PNET_NAME ${PNET_DEFAULT_ARGS})
  CDR(PNET_SOURCES ${PNET_DEFAULT_ARGS})

  set(PNETC_LOCATION pnetc)
  if (TARGET pnetc)
    get_target_property(PNETC_LOCATION pnetc LOCATION)
  endif()

  set ( PNET__default_flags
        --Woverwrite-file=false
        --Wbackup-file=false
        --force-overwrite-file=true
      )

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

  if (NOT IS_ABSOLUTE ${PNET_OUTPUT})
    set(PNET_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PNET_OUTPUT})
  endif()

  if (PNET_COMPONENT)
  else()
    set(PNET_COMPONENT "pnet")
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

  foreach(flag ${PNET_LDFLAGS})
    set (PNET_FLAGS ${PNET_FLAGS} --gen-ldflags=${flag})
  endforeach()

  foreach(flag ${PNET_CXXFLAGS})
    set (PNET_FLAGS ${PNET_FLAGS} --gen-cxxflags=${flag})
  endforeach()

  set(PNETC_ARGS ${PNET__default_flags}
                 ${PNET_FLAGS}
                 ${__pnet_sources}
              -o ${PNET_OUTPUT}
     )

  set (PNET_GEN_OUTPUTS ${PNET_OUTPUT})

  if (PNET_GENERATE)
    set(PNETC_ARGS ${PNETC_ARGS} -g ${PNET_GENERATE})
    set (PNET_GEN_OUTPUTS ${PNET_GEN_OUTPUTS} ${CMAKE_CURRENT_BINARY_DIR}/${PNET_GENERATE})
  endif()

  add_custom_command(OUTPUT ${PNET_GEN_OUTPUTS}
    COMMAND ${PNETC_LOCATION} ${PNETC_ARGS}
    DEPENDS ${PNETC_LOCATION} ${PNET_SOURCES} ${PNET_DEPENDS}
    COMMENT "Building petri-net ${PNET_NAME}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  if (PNET_BUILD)
    # the call to $(MAKE) does currently only work when the gernator is make as
    # well, ninja for example doesn't know about $(MAKE) (and wants it quoted
    # as $$(MAKE) anyways)
    add_custom_command(OUTPUT ${PNET_GEN_OUTPUTS}
      COMMAND "$(MAKE)" -C ${PNET_GENERATE} "BOOST_ROOT=${Boost_INCLUDE_DIR}/../"
      COMMENT "Building modules for petri-net ${PNET_NAME}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      APPEND
      )
  endif()

  add_custom_target (pnet-${PNET_NAME} ALL DEPENDS ${PNET_GEN_OUTPUTS})

  if (PNET_BUILD)
    set_target_properties(pnet-${PNET_NAME} PROPERTIES PNET_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PNET_GENERATE})
  endif()

  set_target_properties(pnet-${PNET_NAME} PROPERTIES LOCATION ${PNET_OUTPUT})
  set_target_properties(pnet-${PNET_NAME} PROPERTIES GEN_OUTPUTS "${PNET_GEN_OUTPUTS}")

  if (PNET_INSTALL)
    install (FILES ${__pnet_sources} ${PNET_OUTPUT} DESTINATION ${PNET_INSTALL} COMPONENT ${PNET_COMPONENT})
    if (PNET_BUILD)
      install(CODE "
         file(GLOB_RECURSE MODULES \"${CMAKE_CURRENT_BINARY_DIR}/${PNET_GENERATE}/*.so\")
         message(STATUS \"  found modules: \${MODULES} \")
         file(INSTALL \${MODULES} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${PNET_INSTALL}/modules\"
                      USE_SOURCE_PERMISSIONS
             )
      " COMPONENT ${PNET_COMPONENT})
    endif()
  endif()
endmacro()
