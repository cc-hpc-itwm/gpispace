# -*- mode: cmake; -*-
set (hdrs
  semaphore.hpp
  event.hpp
  queue.hpp
  channel.hpp
  poll.hpp
  pollable.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util/thread
  COMPONENT headers)
