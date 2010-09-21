set (hdrs
  Config.hpp
  Properties.hpp
  util.hpp
  )

set (srcs
  Config.cpp
  util.cpp
  )

ADD_LIBRARY(sdpa-util ${srcs})
target_link_libraries(sdpa-util ${UUID_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/sdpa/util
  COMPONENT headers)
install(TARGETS sdpa-util
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
