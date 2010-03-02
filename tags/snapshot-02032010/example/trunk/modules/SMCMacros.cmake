# -*- mode: cmake; -*-
#
# This file contains a convenience macro for the use with the SMC state machine compiler
#

## Bei Windows darf --cpp_out kein Laufwerksnamen haben!
##

function(add_state_machine GEN_FSM_C_FILE GEN_FSM_H_FILE FSM_NAME)
  set(_GEN_FSM_H_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}_sm.h" )
  set(_GEN_FSM_C_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}_sm.cpp" )

  message(STATUS "  Compiling statemap ${FSM_NAME}")
  message(STATUS "  Compiling result '${_GEN_FSM_H_FILE}'")

  if(SMC_FOUND)
    IF(WIN32)
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}.sm ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
	COMMAND echo "Compiling statemap ${FSM_NAME}"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm ${CMAKE_CURRENT_BINARY_DIR}/
	COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -serial -c++ ${FSM_NAME}.sm
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
	COMMENT "Compiling '${FSM_NAME}' state machine..."
	)
    else(WIN32)
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}.sm ${_GEN_FSM_H_FILE} ${_GEN_FSM_C_FILE}
      COMMAND echo "Compiling statemap ${FSM_NAME}"
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm ${CMAKE_CURRENT_BINARY_DIR}/
      COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -serial -c++ ${CMAKE_CURRENT_BINARY_DIR}/${FSM_NAME}.sm
      #COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp
      #COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h
      #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_BINARY_DIR}/
      #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h ${CMAKE_CURRENT_BINARY_DIR}/
      #COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -glevel 0 -graph ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      #COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -suffix dot1 -glevel 1 -graph ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      #COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -suffix dot2 -glevel 2 -graph ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      COMMENT "Compiling '${FSM_NAME}' state machine..."
      )
  endIF(WIN32)
    set(${GEN_FSM_H_FILE} ${_GEN_FSM_H_FILE} PARENT_SCOPE)
    set(${GEN_FSM_C_FILE} ${_GEN_FSM_C_FILE} PARENT_SCOPE)

    #add_custom_target(generate_${FSM_NAME} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h)


  else(SMC_FOUND)
    if ("${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp")
      message(FATAL_ERROR "The '${FSM_NAME}' StateMachine needs to be updated but the StateMachineCompiler (SMC) could not be found!")
    endif ("${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp")
  endif(SMC_FOUND)
endfunction(add_state_machine)
