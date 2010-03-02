/*
 * =====================================================================================
 *
 *       Filename:  EventVisitor.hpp
 *
 *    Description:  defines an event visitor
 *
 *        Version:  1.0
 *        Created:  10/29/2009 01:01:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_EVENTS_EVENT_VISITOR_HPP
#define SDPA_EVENTS_EVENT_VISITOR_HPP 1

#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa { namespace events {
  // forward declarations for the event classes
  class CancelJobAckEvent;
  class CancelJobEvent;
  class ConfigNokEvent;
  class ConfigOkEvent;
  class ConfigReplyEvent;
  class ConfigRequestEvent;
  class DeleteJobAckEvent;
  class DeleteJobEvent;
  class ErrorEvent;
  class InterruptEvent;
  class JobFailedAckEvent;
  class JobFailedEvent;
  class JobFinishedAckEvent;
  class JobFinishedEvent;
  class JobResultsReplyEvent;
  class JobStatusReplyEvent;
  class LifeSignEvent;
  class QueryJobStatusEvent;
  class RequestJobEvent;
  class RetrieveJobResultsEvent;
  class RunJobEvent;
  class StartUpEvent;
  class SubmitJobAckEvent;
  class SubmitJobEvent;
  class WorkerRegistrationAckEvent;
  class WorkerRegistrationEvent;

  class EventVisitor
  {
  public:
    virtual ~EventVisitor() {}    

    virtual void visitCancelJobAckEvent(CancelJobAckEvent *) {}
    virtual void visitCancelJobEvent(CancelJobEvent *) {}
    virtual void visitConfigNokEvent(ConfigNokEvent *) {}
    virtual void visitConfigOkEvent(ConfigOkEvent *) {}
    virtual void visitConfigReplyEvent(ConfigReplyEvent *) {}
    virtual void visitConfigRequestEvent(ConfigRequestEvent *) {}
    virtual void visitDeleteJobAckEvent(DeleteJobAckEvent *) {}
    virtual void visitDeleteJobEvent(DeleteJobEvent *) {}
    virtual void visitErrorEvent(ErrorEvent *) {}
    virtual void visitInterruptEvent(InterruptEvent *) {}
    virtual void visitJobFailedAckEvent(JobFailedAckEvent *) {}
    virtual void visitJobFailedEvent(JobFailedEvent *) {}
    virtual void visitJobFinishedAckEvent(JobFinishedAckEvent *) {}
    virtual void visitJobFinishedEvent(JobFinishedEvent *) {}
    virtual void visitJobResultsReplyEvent(JobResultsReplyEvent *) {}
    virtual void visitJobStatusReplyEvent(JobStatusReplyEvent *) {}
    virtual void visitLifeSignEvent(LifeSignEvent *) {}
    virtual void visitQueryJobStatusEvent(QueryJobStatusEvent *) {}
    virtual void visitRequestJobEvent(RequestJobEvent *) {}
    virtual void visitRetrieveJobResultsEvent(RetrieveJobResultsEvent *) {}
    virtual void visitRunJobEvent(RunJobEvent *) {}
    virtual void visitStartUpEvent(StartUpEvent *) {}
    virtual void visitSubmitJobAckEvent(SubmitJobAckEvent *) {}
    virtual void visitSubmitJobEvent(SubmitJobEvent *) {}
    virtual void visitWorkerRegistrationAckEvent(WorkerRegistrationAckEvent *) {}
    virtual void visitWorkerRegistrationEvent(WorkerRegistrationEvent *) {}
  };
}}

#endif
