# -*- mode: cmake; -*-
set (hdrs
  activity.hpp
  flags.hpp
  preference.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/mgmt/type
  COMPONENT headers)
