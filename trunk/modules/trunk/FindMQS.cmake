# -*- mode: cmake; -*-
# Finds the protocol buffers compiler, protoc.
# Protocol Buffers is available at http://code.google.com/apis/protocolbuffers
# This file defines:
# * MQS_FOUND if protoc was found
# * MQS_LIBRARY The lib to link to (currently only a static unix lib, not
# portable) 
# * MQS_INCLUDE_DIR the protoc executable

message(STATUS "FindMQS check")
include(FindPackageHelper)
check_package(MQS mqs/MQSException.hpp mqs 1.0)
