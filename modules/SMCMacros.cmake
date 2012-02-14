# -*- mode: cmake; -*-
#
# This file contains a convenience macro for the use with the SMC state machine compiler
#

## Bei Windows darf --cpp_out kein Laufwerksnamen haben!
##

function(add_state_machine GEN_FSM_C_FILE GEN_FSM_H_FILE FSM_NAME)
  set(_FSM_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm" )
  set(_GEN_FSM_H_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}_sm.h" )
  set(_GEN_FSM_C_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}_sm.cpp" )

  set(_GEN_FSM_H_FALLBACK "${CMAKE_CURRENT_SOURCE_DIR}/generated/${FSM_NAME}_sm.h" )
  set(_GEN_FSM_C_FALLBACK "${CMAKE_CURRENT_SOURCE_DIR}/generated/${FSM_NAME}_sm.cpp" )

  if(SMC_FOUND)
    add_custom_command(
      OUTPUT ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
      COMMAND ${CMAKE_COMMAND} -E copy ${_FSM_FILE} ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -serial -c++ ${FSM_NAME}.sm
      # save the newly generated header and cpp files
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/generated
      COMMAND ${CMAKE_COMMAND} -E copy ${_GEN_FSM_H_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/generated/
      COMMAND ${CMAKE_COMMAND} -E copy ${_GEN_FSM_C_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/generated/
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${_FSM_FILE}
      COMMENT "Compiling '${FSM_NAME}' state machine..."
      )
  else(SMC_FOUND)
    add_custom_command(
      OUTPUT ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
      COMMAND ${CMAKE_COMMAND} -E echo "WARNING: state-machine ${FSM_NAME} needs to be updated but SMC was not found!"
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/generated/${FSM_NAME}_sm.h ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/generated/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${_FSM_FILE}
      COMMENT "Compiling '${FSM_NAME}' state machine... (dummy)"
      )
  endif(SMC_FOUND)

  set(${GEN_FSM_H_FILE} ${_GEN_FSM_H_FILE} PARENT_SCOPE)
  set(${GEN_FSM_C_FILE} ${_GEN_FSM_C_FILE} PARENT_SCOPE)
endfunction(add_state_machine)
