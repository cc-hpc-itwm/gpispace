# -*- mode: cmake; -*-
set (hdrs
  commands.hpp
  descriptor.hpp
  execution_policy.hpp
  queue.hpp
  set.hpp
  signal.hpp
  )

add_we_sources (
  descriptor.cpp
)

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/bits
  COMPONENT headers)
