# -*- mode: cmake; -*-

set (hdrs
  events.hpp
  CancelJobAckEvent.hpp
  CancelJobEvent.hpp
  Codec.hpp
  CodecStrategy.hpp
  ConfigNokEvent.hpp
  ConfigOkEvent.hpp
  ConfigReplyEvent.hpp
  ConfigRequestEvent.hpp
  DeleteJobAckEvent.hpp
  DeleteJobEvent.hpp
  ErrorEvent.hpp
  EventHandler.hpp
  id_generator.hpp
  InterruptEvent.hpp
  JobEvent.hpp
  JobFailedAckEvent.hpp
  JobFailedEvent.hpp
  JobFinishedAckEvent.hpp
  JobFinishedEvent.hpp
  JobResultsReplyEvent.hpp
  JobStatusReplyEvent.hpp
  LifeSignEvent.hpp
  MgmtEvent.hpp
  QueryJobStatusEvent.hpp
  RequestJobEvent.hpp
  RetrieveJobResultsEvent.hpp
  RunJobEvent.hpp
  SDPAEvent.hpp
  sdpa-msg.proto
  Serialization.hpp
  StartUpEvent.hpp
  SubmitJobAckEvent.hpp
  SubmitJobEvent.hpp
  WorkerRegistrationAckEvent.hpp
  WorkerRegistrationEvent.hpp
  )

set (srcs
  ErrorEvent.cpp
  SDPAEvent.cpp
  )

ADD_LIBRARY(sdpa-events ${srcs})
