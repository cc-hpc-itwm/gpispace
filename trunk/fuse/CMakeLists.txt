
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_FUSE QUIET fuse)

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

add_executable(fuse.hello fuse.hello.c)
target_link_libraries (fuse.hello ${FUSE_LIBRARIES})
set_target_properties(fuse.hello PROPERTIES
    COMPILE_FLAGS ${PC_FUSE_CFLAGS_OTHER}
  )
