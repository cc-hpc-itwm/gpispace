add_subdirectory(util)

add_subdirectory(daemon)

add_library (sdpa-client client.cpp)

set (hdrs
  common.hpp
  JobId.hpp
  logging.hpp
  SDPAException.hpp
  types.hpp
  job_states.hpp
  id_generator.hpp
)

set (srcs
  JobId.cpp
)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})

add_subdirectory (com)
