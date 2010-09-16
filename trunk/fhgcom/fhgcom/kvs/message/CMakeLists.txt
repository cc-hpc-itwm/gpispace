# -*- mode: cmake; -*-

set (hdrs
  put.hpp
  get.hpp
  del.hpp
  error.hpp
  load.hpp
  save.hpp
  list.hpp
  req_list.hpp
  clear.hpp

  type.hpp
  show.hpp
)

install(FILES ${hdrs} DESTINATION include/fhgcom/kvs/message)
