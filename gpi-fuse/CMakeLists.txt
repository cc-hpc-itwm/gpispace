find_package (PkgConfig QUIET)
pkg_check_modules (PC_FUSE fuse QUIET)

find_path ( FUSE_INCLUDE_DIR fuse/fuse.h
            HINTS ${PC_FUSE_INCLUDE_DIRS} ${PC_FUSE_INCLUDEDIR}
            PATH_SUFFIXES fuse
          )

foreach (lib ${PC_FUSE_LIBRARIES})
  find_library ( FUSE_LIBRARY_${lib} NAMES ${lib}
                 HINTS ${PC_FUSE_LIBDIR} ${PC_FUSE_LIBRARY_DIRS}
	       )
  set (FUSE_LIBRARIES ${FUSE_LIBRARIES} ${FUSE_LIBRARY_${lib}})
endforeach()

if (NOT FUSE_INCLUDE_DIR)
  message (FATAL_ERROR "FUSE NOT FOUND!")
endif (NOT FUSE_INCLUDE_DIR)
include_directories (SYSTEM ${FUSE_INCLUDE_DIR})

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (GPISpace REQUIRED QUIET)
include_directories (${GPI_SPACE_INCLUDE_DIR})

find_package (FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

find_package (UTIL REQUIRED QUIET)
include_directories (${UTIL_INCLUDE_DIR})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_library (gpi-fuse gpifs/comm.cpp)

install (TARGETS gpi-fuse
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries
)

add_executable (gpifs gpifs/gpifs.cpp)

target_link_libraries (gpifs gpi-fuse)
target_link_libraries (gpifs ${FUSE_LIBRARIES})
target_link_libraries (gpifs ${GPI_SPACE_LIBRARY})
target_link_libraries (gpifs ${FhgLog_LIBRARY})
target_link_libraries (gpifs ${Boost_LIBRARIES})
set_target_properties (gpifs PROPERTIES COMPILE_FLAGS "${PC_FUSE_CFLAGS_OTHER}")

install (TARGETS gpifs
  RUNTIME
  DESTINATION bin
  COMPONENT runtime
)

fhg_add_test (test/test_split.cpp
  PROJECT test_gpi-fuse
  LINK_LIBRARIES gpi-fuse
                 ${FUSE_LIBRARIES}
                 ${GPI_SPACE_LIBRARY}
                 ${FhgLog_LIBRARY}
  COMPILE_FLAGS "${PC_FUSE_CFLAGS_OTHER}"
)

add_executable (seek test/seek.c)
set_target_properties (seek PROPERTIES COMPILE_FLAGS -std=c99)
