# Install script for directory: /amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/orchFSM

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sdpa/orchFSM" TYPE FILE FILES
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/Job.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/JobImpl.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/memory.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/LoggingConfigurator.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/logging.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/Token.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/Activity.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/Parameter.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/Properties.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/SDPAException.hpp"
    "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/common.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/amd/fs5/root/home/u/rotaru/projectsc++/sdpa/sdpa/branches/rotaru/sdpa/orchFSM/liborchFSM.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" MATCHES "^(Unspecified)$")

