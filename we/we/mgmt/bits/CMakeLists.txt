# -*- mode: cmake; -*-
set (hdrs
  commands.hpp
  descriptor.hpp
  execution_policy.hpp
  policy.hpp
  queue.hpp
  set.hpp
  signal.hpp
  traits.hpp
  transition_visitors.hpp
  types.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/bits
  COMPONENT headers)
