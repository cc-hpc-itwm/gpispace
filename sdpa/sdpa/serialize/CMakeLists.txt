# -*- mode: cmake; -*-
set (hdrs
  unordered_map.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/serialize
  COMPONENT headers)
