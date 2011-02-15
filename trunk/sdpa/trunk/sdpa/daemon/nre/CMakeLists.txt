# -*- mode: cmake; -*-

set (hdrs
  	NREFactory.hpp
  	NRE.hpp
  	BasicWorkerClient.hpp
  	SchedulerNRE.hpp
)

if (USE_REAL_WE)
  add_subdirectory(nre-worker)
endif (USE_REAL_WE)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/nre
  COMPONENT headers)
