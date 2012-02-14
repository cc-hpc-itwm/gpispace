# -*- mode: cmake; -*-
set (hdrs
  unordered_map.hpp
  unordered_set.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/serialize
  COMPONENT headers)
