cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (gpi-fuse)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_FUSE fuse)

FIND_PATH( FUSE_INCLUDE_DIR fuse/fuse.h
           HINTS
           ${PC_FUSE_INCLUDE_DIRS}
           ${PC_FUSE_INCLUDEDIR}
           PATH_SUFFIXES fuse
          )

FOREACH (lib ${PC_FUSE_LIBRARIES})
  FIND_LIBRARY( FUSE_LIBRARY_${lib} NAMES ${lib}
                HINTS
                ${PC_FUSE_LIBDIR}
                ${PC_FUSE_LIBRARY_DIRS}
	      )

  SET(FUSE_LIBRARIES ${FUSE_LIBRARIES} ${FUSE_LIBRARY_${lib}})
ENDFOREACH (lib ${PC_FUSE_LIBRARIES})

message (STATUS "FUSE_INCLUDE_DIR = ${FUSE_INCLUDE_DIR}")
message (STATUS "FUSE_LIBRARIES = ${FUSE_LIBRARIES}")

if (NOT FUSE_INCLUDE_DIR)
  message (FATAL_ERROR "FUSE NOT FOUND!")
endif (NOT FUSE_INCLUDE_DIR)

include_directories (${FUSE_INCLUDE_DIR})

add_subdirectory(gpifs)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test)
endif (BUILD_TESTING)
