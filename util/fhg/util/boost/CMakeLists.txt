# bernd.loerwald@itwm.fraunhofer.de

set (hdrs
  optional.hpp
)

set (srcs
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/boost
  COMPONENT headers)
