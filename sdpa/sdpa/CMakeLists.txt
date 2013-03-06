add_subdirectory(util)
add_subdirectory(events)

add_subdirectory(daemon)
add_subdirectory(client)

set (hdrs
  common.hpp
  JobId.hpp
  logging.hpp
  memory.hpp
  SDPAException.hpp
  types.hpp
  uuidgen.hpp
  uuid.hpp
  job_states.hpp
)

set (srcs
  JobId.cpp
  uuidgen.cpp
)

ADD_LIBRARY(sdpa ${hdrs} ${srcs})
#target_link_libraries(sdpa seda)      # ${SEDA_LIBRARY}
#target_link_libraries(sdpa ${FhgLog_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/sdpa
  COMPONENT headers)

install(TARGETS sdpa
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)

add_subdirectory (com)
