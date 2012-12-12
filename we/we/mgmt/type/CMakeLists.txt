# -*- mode: cmake; -*-
set (hdrs
  activity.hpp
  flags.hpp
  )

set (srcs
  flags.cpp
  activity.cpp
)

add_library (we_mgmt_type ${srcs})
add_we_library (we_mgmt_type)

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/type
  COMPONENT headers)
