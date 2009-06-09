#
# This file contains a convenience macro for using Googles Protocol Buffers. Generate
# Serialization and Deserialization of complex data structures.
#
# Please specify the protoname without its extenstion, e.g. adressbook for a file adressbook.pb
function(add_protocol_buffers PROTONAME)
  if(PB_FOUND)
    if ("${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.proto" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.cc")
        add_custom_command(
          OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.h
          COMMAND ${PB_PROTOC_CMD} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${CMAKE_CURRENT_SOURCE_DIR} --python_out=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.proto
          COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.cc
          COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.h
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.proto
          COMMENT "Creating C++ bindings for '${PROTONAME}.proto' protocol buffer..."
        )
    endif ("${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.proto" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.cc")
  else(PB_FOUND)
    if ("${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.proto" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.cc")
      message(FATAL_ERROR "The '${PROTONAME}.proto' protocol buffer needs to be updated but the protocol buffer compiler (protoc) could not be found!")
    endif ("${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.proto" IS_NEWER_THAN "${CMAKE_CURRENT_SOURCE_DIR}/${PROTONAME}.pb.cc")
  endif(PB_FOUND)
endfunction()
