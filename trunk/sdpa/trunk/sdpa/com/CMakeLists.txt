set (hdrs
  manager.hpp
  manager.tcc
  )

set (srcs
  manager.cpp
  )

ADD_LIBRARY(sdpa-com ${srcs} ${hdrs})
target_link_libraries(sdpa-com fhgcom)

install(FILES ${hdrs} DESTINATION include/sdpa/com)
install(TARGETS sdpa-com ARCHIVE DESTINATION lib)
