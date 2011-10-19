cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (ufbmig)

option(WITH_UFBMIG_DAEMON "Compile UfbMigDaemon" OFF)

if (WITH_UFBMIG_DAEMON)
  add_subdirectory(daemon)
endif()
