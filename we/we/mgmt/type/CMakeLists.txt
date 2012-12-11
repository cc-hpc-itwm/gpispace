# -*- mode: cmake; -*-
set (hdrs
  activity.hpp
  flags.hpp
  requirement.hpp
  preference.hpp
  )

set (srcs
  flags.cpp
)

add_library (we_mgmt_type ${srcs})
add_we_library (we_mgmt_type)

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/type
  COMPONENT headers)
