ADD_LIBRARY(fhgcom
  connection.cpp
  header.cpp
  peer.cpp
)
target_link_libraries (fhgcom fhglog Util::Generic)

  add_subdirectory (tests)
