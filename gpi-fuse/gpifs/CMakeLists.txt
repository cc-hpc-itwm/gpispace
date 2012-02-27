set(hdrs
  alloc.hpp
  comm.hpp
  file.hpp
  id.hpp
  log.hpp
  segment.hpp
  splitted_path.hpp
  state.hpp
  util.hpp
  )

set(srcs
  comm.cpp
  )

add_library(gpi-fuse ${hdrs} ${srcs})

add_executable(gpifs gpifs.cpp ${srcs})

set(targets gpifs)
foreach (target ${targets})
  if(APPLE)
    STRING(REGEX REPLACE ";" " " PC_FUSE_CFLAGS_OTHER "${PC_FUSE_CFLAGS_OTHER}")
  endif(APPLE)
  target_link_libraries (${target} gpi-fuse)
  target_link_libraries (${target} ${FUSE_LIBRARIES})
  target_link_libraries (${target} ${GPI_SPACE_LIBRARY})
  target_link_libraries (${target} ${FhgLog_LIBRARY})
  target_link_libraries (${target} ${Boost_LIBRARIES})
  set_target_properties(${target} PROPERTIES
      COMPILE_FLAGS "${PC_FUSE_CFLAGS_OTHER}"
    )
endforeach (target ${targets})

install(TARGETS gpi-fuse
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)

install(TARGETS gpifs
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)

install(FILES ${hdrs}
  DESTINATION include/gpifs
  COMPONENT headers
  )
