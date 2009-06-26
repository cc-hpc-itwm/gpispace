# -*- mode: cmake; -*-
#
# This file contains a convenience macro for the use with the SMC state machine compiler
#

## Bei Windows darf --cpp_out kein Laufwerksnamen haben!
##

function(add_state_machine FSM_NAME)
  set(${FSM_NAME}_H "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h")
  set(${FSM_NAME}_C "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp")

  if(SMC_FOUND)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h
      COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -c++ ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp
      COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_BINARY_DIR}/
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h ${CMAKE_CURRENT_BINARY_DIR}/
      #COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -glevel 0 -graph ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      #COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -suffix dot1 -glevel 1 -graph ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      #COMMAND ${JAVA_RUNTIME} -jar ${SMC_JAR} -suffix dot2 -glevel 2 -graph ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm
      COMMENT "Compiling '${FSM_NAME}' state machine...")
    add_custom_target(generate_${FSM_NAME} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.h)

  else(SMC_FOUND)
    if ("${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp")
      message(FATAL_ERROR "The '${FSM_NAME}' StateMachine needs to be updated but the StateMachineCompiler (SMC) could not be found!")
    endif ("${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}.sm" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${FSM_NAME}_sm.cpp")
  endif(SMC_FOUND)
endfunction(add_state_machine)
