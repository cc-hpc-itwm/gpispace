# -*- mode: cmake; -*-
set (hdrs
  semaphore.hpp
  event.hpp
  queue.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/thread
  COMPONENT headers)
