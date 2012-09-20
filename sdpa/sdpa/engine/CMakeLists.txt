add_subdirectory(plugin)

set (hdrs
)

install (FILES ${hdrs}
  DESTINATION include/sdpa/engine
  COMPONENT headers)
