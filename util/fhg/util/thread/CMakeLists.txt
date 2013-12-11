set (hdrs
  event.hpp
  queue.hpp
  channel.hpp
  pool.hpp
  set.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/thread
  COMPONENT headers)
