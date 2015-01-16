include_directories (${FhgLog_INCLUDE_DIR})

set (srcs
  session.cpp
  peer.cpp
  connection.cpp

  header.cpp
)

ADD_LIBRARY(fhgcom ${srcs})
ADD_LIBRARY(fhgcom-shared SHARED ${srcs})
target_link_libraries (fhgcom fhglog)
target_link_libraries (fhgcom-shared fhglog-shared)
set_target_properties(fhgcom-shared PROPERTIES OUTPUT_NAME fhgcom)

  add_subdirectory (tests)
