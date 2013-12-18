# -*- mode: cmake; -*-
set (hdrs
  IEvent.hpp
  Stage.hpp
)

set (srcs
  Stage.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
