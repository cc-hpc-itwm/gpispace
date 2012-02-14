# -*- mode: cmake; -*-
set (hdrs
  bind.hpp
  container.hpp
  exception.hpp
  show.hpp
  type.hpp
  value.hpp
)
install(FILES ${hdrs}
  DESTINATION include/we/type/value/container
  COMPONENT headers)
