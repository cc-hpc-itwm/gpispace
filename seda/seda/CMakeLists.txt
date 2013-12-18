# -*- mode: cmake; -*-
set (hdrs
  DiscardStrategy.hpp
  EventCountStrategy.hpp
  IEvent.hpp
  logging.hpp
  shared_ptr.hpp
  Stage.hpp
  StrategyDecorator.hpp
  Strategy.hpp
)

set (srcs
  Stage.cpp
  StrategyDecorator.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
