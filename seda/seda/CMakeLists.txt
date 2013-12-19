# -*- mode: cmake; -*-
set (hdrs
  IEvent.hpp
  Stage.hpp
)

set (srcs
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
