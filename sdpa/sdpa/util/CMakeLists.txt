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

install(FILES ${hdrs}
  DESTINATION include/sdpa/util
  COMPONENT headers)
install(TARGETS sdpa-util
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
