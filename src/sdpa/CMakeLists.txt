ADD_LIBRARY(sdpa
  capability.cpp
  client.cpp
  com/NetworkStrategy.cpp
  daemon/Agent.cpp
  daemon/Job.cpp
  daemon/scheduler/CoallocationScheduler.cpp
  daemon/scheduler/Reservation.cpp
  daemon/Worker.cpp
  daemon/WorkerManager.cpp
  events/Codec.cpp
)

target_link_libraries (sdpa
  IML::vmem-pc-client-static
  IML::vmem-pc-segment
  RPC
  Boost::serialization
  Boost::date_time
  fhgcom
  Boost::thread
  fhg-util
  pnet
  Util::Generic
  gspc::logging
)

add_subdirectory (test)
add_subdirectory (test/bin)
add_subdirectory (test/sdpa)
