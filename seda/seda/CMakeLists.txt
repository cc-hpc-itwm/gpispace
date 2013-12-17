# -*- mode: cmake; -*-
set (hdrs
  common.hpp
  constants.hpp
  DiscardStrategy.hpp
  EventCountStrategy.hpp
  EventQueue.hpp
  ForwardStrategy.hpp
  IEvent.hpp
  IEventQueue.hpp
  logging.hpp
  SedaException.hpp
  shared_ptr.hpp
  StageFactory.hpp
  Stage.hpp
  StageRegistry.hpp
  StageWorker.hpp
  StrategyDecorator.hpp
  Strategy.hpp
  StringEvent.hpp
  SystemEvent.hpp
  UserEvent.hpp
)

set (srcs
  constants.cpp
  ForwardStrategy.cpp
  Stage.cpp
  StageRegistry.cpp
  StageWorker.cpp
  StrategyDecorator.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
