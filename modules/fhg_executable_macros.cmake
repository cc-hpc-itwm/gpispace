include(cmake_parse_arguments)
include(car_cdr_macros)

macro (FHG_ADD_RUNTIME_EXECUTABLE)
  PARSE_ARGUMENTS(EXECUTABLE "LINK_LIBRARIES;NAME" "" ${ARGN})
  CAR (EXECUTABLE_SOURCE ${EXECUTABLE_DEFAULT_ARGS})
  CDR (EXECUTABLE_ADDITIONAL_SOURCES ${EXECUTABLE_DEFAULT_ARGS})

  if (EXECUTABLE_NAME)
    set (exe_name ${EXECUTABLE_NAME})
  else()
    string (REGEX REPLACE "(.*)\\.c.*" "\\1" exe_name ${EXECUTABLE_SOURCE})
  endif()

  add_executable (
    ${exe_name}
    ${EXECUTABLE_SOURCE} ${EXECUTABLE_ADDITIONAL_SOURCES}
  )

  target_link_libraries (${exe_name} ${EXECUTABLE_LINK_LIBRARIES})

  install (TARGETS ${exe_name} RUNTIME DESTINATION bin)

  get_target_property (exe_target_location ${exe_name} LOCATION)

  install(CODE "
    execute_process(COMMAND ${CMAKE_SOURCE_DIR}/bundle/bundle.sh
                            -p \"\${CMAKE_INSTALL_PREFIX}\"
                            -d
                            -L \"\${CMAKE_INSTALL_PREFIX}/lib\"
                            -x libibverbs.*
                            -x libxcb.*
                            -x libSM.*
                            -x libc.so.*
                            -x libz.so.*
                            -x libm.so.*
                            -x librt.*
                            -x libfont.*
                            -x libfreetype.*
                            -x libaudio.*
                            -x libICE.*
                            -x libglib.*
                            -x libgobject.*
                            -x libdl.*
                            -x libX.*so
                            -x libpthread.*
                            -x libgthread.*
                            -x libreadline.*
                            -x libboost*.*
                             ${exe_target_location}
                    RESULT_VARIABLE __res
                    ERROR_VARIABLE __err
    )
    if (NOT \${__res} EQUAL 0)
       message(FATAL_ERROR \"Could not bundle dependencies: \${__err}\")
    endif()
    " )
endmacro()
