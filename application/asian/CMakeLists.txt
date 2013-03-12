# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

SET(SRCS asianopt.cpp basics.cpp interface.cpp prob.cpp random.cpp ) #asianAPI.cpp)
SET(HDRS asianopt.h basics.h prob.h random.h asian.h ) #asianAPI.h)

# SET(SRCS2 ${SRCS} asian_spu.cpp )
SET(SRCS2 ${SRCS}  )
SET(SRCS3 ${SRCS} mcbeispiel.cpp )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Asian ${SRCS3})

ADD_LIBRARY(asian_x86 STATIC ${SRCS2})

add_subdirectory(pnet)

install(TARGETS Asian
  RUNTIME
  DESTINATION ${SDPA_APPS_DIR}/asian/bin
  COMPONENT applications)
