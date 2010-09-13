# -*- mode: cmake; -*-
#
# This file contains a convenience macro for the use with the SMC state machine compiler
#

## Bei Windows darf --cpp_out kein Laufwerksnamen haben!
##

function(add_state_machine GEN_FSM_C_FILE GEN_FSM_H_FILE FSM_NAME)
  set(_GEN_FSM_H_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h" )
  set(_GEN_FSM_C_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp" )

  message(STATUS "  Compiling statemap ${FSM_NAME}")
  message(STATUS "  Compiling result '${_GEN_FSM_H_FILE}'")

  if(SMC_FOUND)
    IF(WIN32)
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
	COMMAND echo "Compiling statemap ${FSM_NAME}"
#	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm ${CMAKE_CURRENT_BINARY_DIR}/
	COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -serial -c++ ${FSM_NAME}.sm
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	COMMENT "Compiling '${FSM_NAME}' state machine..."
	)
    else(WIN32)
      add_custom_command(
	OUTPUT ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
	COMMAND echo "Compiling statemap ${FSM_NAME}"
	COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -serial -c++ ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	COMMENT "Compiling '${FSM_NAME}' state machine..."
      )
  endIF(WIN32)

    #add_custom_target(generate_${FSM_NAME} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h)

  else(SMC_FOUND)
    add_custom_command(
      OUTPUT ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
      COMMAND echo "WARNING: state-machine ${FSM_NAME} must be updated but SMC was not found!"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      COMMENT "Compiling '${FSM_NAME}' state machine..."
      )
  endif(SMC_FOUND)

  message (STATUS "generated files: ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}")

  set(${GEN_FSM_H_FILE} ${_GEN_FSM_H_FILE} PARENT_SCOPE)
  set(${GEN_FSM_C_FILE} ${_GEN_FSM_C_FILE} PARENT_SCOPE)
endfunction(add_state_machine)
