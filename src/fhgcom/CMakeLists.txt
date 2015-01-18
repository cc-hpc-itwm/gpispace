set (srcs
  session.cpp
  peer.cpp
  connection.cpp

  header.cpp
)

ADD_LIBRARY(fhgcom ${srcs})
target_link_libraries (fhgcom fhglog)

  add_subdirectory (tests)
