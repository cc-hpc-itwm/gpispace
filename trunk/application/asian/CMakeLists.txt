# -*- mode: cmake; -*-
include_directories(${CMAKE_SOURCE_DIR}/include)

SET(SRCS asianopt.cpp basics.cpp interface.cpp prob.cpp random.cpp ) #asianAPI.cpp)
SET(HDRS asianopt.h basics.h prob.h random.h asian.h ) #asianAPI.h)

# SET(SRCS2 ${SRCS} asian_spu.cpp )
SET(SRCS2 ${SRCS}  )
SET(SRCS3 ${SRCS} mcbeispiel.cpp )

include_directories(.)

add_executable(Asian ${SRCS3})

ADD_LIBRARY(asian_x86 STATIC ${SRCS2})

install(TARGETS Asian
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
