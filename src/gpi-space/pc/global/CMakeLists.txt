ADD_LIBRARY(gpi-space-pc-global topology.cpp)
target_link_libraries(gpi-space-pc-global
  fhgcom
  Util::Generic
)
