# -*- mode: cmake; -*-
set (hdrs
  commands.hpp
  descriptor.hpp
  execution_policy.hpp
  queue.hpp
  set.hpp
  signal.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/bits
  COMPONENT headers)
