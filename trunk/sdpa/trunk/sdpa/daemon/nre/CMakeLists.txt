# -*- mode: cmake; -*-

set (hdrs
  Codec.hpp
  ExecutionContext.hpp
  messages.hpp
 # NREFactory.hpp
  NRE.hpp
  NreWorkerClient.hpp
  SchedulerNRE.hpp
  Serialization.hpp
  )

set (hdrs
  )

add_subdirectory(nre-worker)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/nre
  COMPONENT headers)
