add_executable(gpifs gpifs.cpp)

set(targets gpifs)

foreach (target ${targets})
  if(APPLE)
    STRING(REGEX REPLACE ";" " " PC_FUSE_CFLAGS_OTHER "${PC_FUSE_CFLAGS_OTHER}")
  endif(APPLE)
  target_link_libraries (${target} ${FUSE_LIBRARIES})
  set_target_properties(${target} PROPERTIES
      COMPILE_FLAGS "${PC_FUSE_CFLAGS_OTHER}"
    )
endforeach (target ${targets})
