# -*- mode: cmake; -*-
set (hdrs
  commands.hpp
  descriptor.hpp
  execution_policy.hpp
  queue.hpp
  set.hpp
  signal.hpp
  )

set(srcs
  descriptor.cpp
)

add_library (we_mgmt_bits ${srcs})
add_we_library (we_mgmt_bits)

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/bits
  COMPONENT headers)
