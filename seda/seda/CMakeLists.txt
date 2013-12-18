# -*- mode: cmake; -*-
set (hdrs
  DiscardStrategy.hpp
  EventCountStrategy.hpp
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
  Stage.cpp
  StageRegistry.cpp
  StrategyDecorator.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
