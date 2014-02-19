set (hdrs
  itopology.hpp
  topology.hpp
)

set (srcs
  topology.cpp
)

ADD_LIBRARY(gpi-space-pc-global ${hdrs} ${srcs})
target_link_libraries(gpi-space-pc-global
  ${FhgCom_LIBRARY}
)
