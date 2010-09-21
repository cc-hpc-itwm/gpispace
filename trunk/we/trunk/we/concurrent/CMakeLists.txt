# -*- mode: cmake; -*-
set (hdrs
  deque.hpp
  net.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/concurrent
  COMPONENT headers)
