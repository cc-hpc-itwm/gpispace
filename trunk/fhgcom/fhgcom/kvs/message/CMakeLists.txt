# -*- mode: cmake; -*-

set (hdrs
  put.hpp
  get.hpp
  del.hpp
  value.hpp
  error.hpp
  load.hpp
  save.hpp

  type.hpp
  show.hpp
)

install(FILES ${hdrs} DESTINATION include/fhgcom/kvs/message)
