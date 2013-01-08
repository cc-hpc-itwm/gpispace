# -*- mode: cmake; -*-
set (hdrs
  activity.hpp
  flags.hpp
  )

add_we_sources (
  flags.cpp
  activity.cpp
)

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/type
  COMPONENT headers)
