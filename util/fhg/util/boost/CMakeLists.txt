# bernd.loerwald@itwm.fraunhofer.de

set (hdrs
  optional.hpp
  test.hpp
  tuple.hpp
  variant.hpp
)

set (srcs
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/boost
  COMPONENT headers)
