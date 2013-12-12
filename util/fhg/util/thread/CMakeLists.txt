set (hdrs
  event.hpp
  queue.hpp
  pool.hpp
  set.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/thread
  COMPONENT headers)
