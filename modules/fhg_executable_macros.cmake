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
endmacro()
