include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (hdrs
  message.hpp
  manager.hpp
  manager.tcc
  connection.hpp
  )

set (srcs
  manager.cpp
#  connection.cpp
  )

ADD_LIBRARY(sdpa-com ${srcs} ${hdrs})
target_link_libraries(sdpa-com fhgcom)

install(FILES ${hdrs}
  DESTINATION include/sdpa/com
  COMPONENT headers)
install(TARGETS sdpa-com
  ARCHIVE DESTINATION lib
  COMPONENT libraries)
