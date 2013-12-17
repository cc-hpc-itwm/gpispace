# -*- mode: cmake; -*-
set (hdrs
  AccumulateStrategy.hpp
  common.hpp
  CompositeStrategy.hpp
  constants.hpp
  DiscardStrategy.hpp
  ErrorEvent.hpp
  EventCountStrategy.hpp
  EventDecorator.hpp
  EventFactory.hpp
  EventNotSupported.hpp
  EventQueue.hpp
  EventPrioQueue.hpp
  FilterStrategy.hpp
  ForwardStrategy.hpp
  IEvent.hpp
  IEventQueue.hpp
  logging.hpp
  LoggingStrategy.hpp
  LossyDaemonStrategy.hpp
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
  AccumulateStrategy.cpp
  CompositeStrategy.cpp
  constants.cpp
  ErrorEvent.cpp
  ForwardStrategy.cpp
  LossyDaemonStrategy.cpp
  Stage.cpp
  StageRegistry.cpp
  StageWorker.cpp
  StrategyDecorator.cpp
  Timer.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})
target_link_libraries(seda ${FhgLog_LIBRARY})
