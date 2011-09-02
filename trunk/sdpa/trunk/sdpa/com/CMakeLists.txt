include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
  NetworkStrategy.hpp
  )

set (srcs
  NetworkStrategy.cpp
  )

ADD_LIBRARY(sdpa-com ${srcs} ${hdrs})
target_link_libraries(sdpa-com ${FhgCom_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/sdpa/com
  COMPONENT headers)
install(TARGETS sdpa-com
  ARCHIVE DESTINATION lib
  COMPONENT libraries)
