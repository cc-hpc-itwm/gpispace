include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
  NetworkStrategy.hpp
  )

set (srcs
  NetworkStrategy.cpp
  )

ADD_LIBRARY(sdpa-com ${srcs} ${hdrs})
target_link_libraries(sdpa-com fhgcom)

install(FILES ${hdrs}
  DESTINATION include/sdpa/com
  COMPONENT headers)
install(TARGETS sdpa-com
  ARCHIVE DESTINATION lib
  COMPONENT libraries)
