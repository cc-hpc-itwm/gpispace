# Reflect the package structure

set (hdrs
    messages.hpp
	Codec.hpp
 	ExecutionContext.hpp
  	Serialization.hpp
  	NreWorkerClient.hpp
)
  
install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon/nre/nre-worker
  COMPONENT headers)
  
add_subdirectory(nre-worker)
