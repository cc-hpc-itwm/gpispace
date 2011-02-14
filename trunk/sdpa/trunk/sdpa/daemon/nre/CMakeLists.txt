# -*- mode: cmake; -*-

set (hdrs
  Codec.hpp
  ExecutionContext.hpp
  messages.hpp
  NREFactory.hpp
  NRE.hpp
  BasicWorkerClient.hpp
if (USE_REAL_WE)
  NreWorkerClient.hpp
endif (USE_REAL_WE)
  SchedulerNRE.hpp
  Serialization.hpp
)
  
if (USE_REAL_WE)
  add_subdirectory(nre-worker)
endif (USE_REAL_WE)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/nre
  COMPONENT headers)
