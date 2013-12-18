# -*- mode: cmake; -*-
set (hdrs
  IEvent.hpp
  logging.hpp
  Stage.hpp
  Strategy.hpp
)

set (srcs
  Stage.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
