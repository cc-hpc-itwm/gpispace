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

  add_custom_command (OUTPUT "${CMAKE_BINARY_DIR}/bundle-${exe_name}"
    COMMAND "${CMAKE_SOURCE_DIR}/bundle/bundle.sh"
    ARGS -o "${CMAKE_BINARY_DIR}/bundle-${exe_name}"
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
         $<TARGET_FILE:${exe_name}>
    DEPENDS ${exe_name}
  )
  add_custom_target (${exe_name}-bundled-libraries ALL
    DEPENDS "${CMAKE_BINARY_DIR}/bundle-${exe_name}"
  )

  install (DIRECTORY "${CMAKE_BINARY_DIR}/bundle-${exe_name}/"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    USE_SOURCE_PERMISSIONS
  )
endmacro()
