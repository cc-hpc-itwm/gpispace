# Install script for directory: /amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sdpa/events" TYPE FILE FILES
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/InternalEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/ErrorEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/JobEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/SubmitJobEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/RunJobEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/CancelJobEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/CancelJobAckEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/FailJobEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/JobFinishedEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/QueryJobStatusEvent.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/JobStatusAnswerEvent.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/events/libsdpa-events.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")

