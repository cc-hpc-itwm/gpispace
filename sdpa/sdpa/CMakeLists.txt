add_subdirectory(daemon)

set (hdrs
  common.hpp
  JobId.hpp
  logging.hpp
  types.hpp
  job_states.hpp
  id_generator.hpp
  com/NetworkStrategy.hpp
)

set (srcs
  client.cpp
  JobId.cpp
  com/NetworkStrategy.cpp
)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})
target_link_libraries (sdpa ${Boost_SERIALIZATION_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${FhgCom_LIBRARY} ${Boost_THREAD_LIBRARY})
