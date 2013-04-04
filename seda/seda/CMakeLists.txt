# -*- mode: cmake; -*-
set (hdrs
  AccumulateStrategy.hpp
  common.hpp
  CompositeStrategy.hpp
  constants.hpp
  CoutStrategy.hpp
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
  util.hpp
)

set (srcs
  AccumulateStrategy.cpp
  common.cpp
  CompositeStrategy.cpp
  constants.cpp
  CoutStrategy.cpp
  ErrorEvent.cpp
  ForwardStrategy.cpp
  LossyDaemonStrategy.cpp
  Stage.cpp
  StageRegistry.cpp
  StageWorker.cpp
  StrategyDecorator.cpp
  Timer.cpp
  util.cpp
  )

ADD_LIBRARY(seda STATIC ${srcs})

if (WITH_LOG4CPP AND LOG4CPP_FOUND)
  target_link_libraries(seda ${LOG4CPP_LIBRARY})
endif (WITH_LOG4CPP AND LOG4CPP_FOUND)

if (WITH_FHGLOG AND FhgLog_FOUND)
  target_link_libraries(seda ${FhgLog_LIBRARY})
endif (WITH_FHGLOG AND FhgLog_FOUND)

target_link_libraries(seda ${Boost_LIBRARIES})
