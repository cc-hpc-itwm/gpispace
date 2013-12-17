# -*- mode: cmake; -*-
set (hdrs
  common.hpp
  constants.hpp
  DiscardStrategy.hpp
  ErrorEvent.hpp
  EventCountStrategy.hpp
  EventDecorator.hpp
  EventFactory.hpp
  EventNotSupported.hpp
  EventQueue.hpp
  EventPrioQueue.hpp
  ForwardStrategy.hpp
  IEvent.hpp
  IEventQueue.hpp
  logging.hpp
  SedaException.hpp
  shared_ptr.hpp
  StageAlreadyRegistered.hpp
  StageFactory.hpp
  Stage.hpp
  StageNotFound.hpp
  StageRegistry.hpp
  StageWorker.hpp
  StrategyDecorator.hpp
  Strategy.hpp
  StringEvent.hpp
  Synchronizable.hpp
  SystemEvent.hpp
  TimerEvent.hpp
  Timer.hpp
  UserEvent.hpp
)

set (srcs
  constants.cpp
  ErrorEvent.cpp
  ForwardStrategy.cpp
  Stage.cpp
  StageRegistry.cpp
  StageWorker.cpp
  StrategyDecorator.cpp
  Timer.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
