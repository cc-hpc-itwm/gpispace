# -*- mode: cmake; -*-
set (hdrs
  common.hpp
  DiscardStrategy.hpp
  EventCountStrategy.hpp
  EventQueue.hpp
  ForwardStrategy.hpp
  IEvent.hpp
  logging.hpp
  SedaException.hpp
  shared_ptr.hpp
  Stage.hpp
  StageRegistry.hpp
  StrategyDecorator.hpp
  Strategy.hpp
)

set (srcs
  ForwardStrategy.cpp
  Stage.cpp
  StageRegistry.cpp
  StrategyDecorator.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
