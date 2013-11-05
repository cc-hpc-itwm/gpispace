#ifndef SDPA_EVENT_HANDLER_HPP
#define SDPA_EVENT_HANDLER_HPP 1

namespace sdpa
{
  namespace events
  {
    class CancelJobAckEvent;
    class CancelJobEvent;
    class DeleteJobAckEvent;
    class DeleteJobEvent;
    class ErrorEvent;
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
    class SubmitJobAckEvent;
    class SubmitJobEvent;
    class WorkerRegistrationAckEvent;
    class WorkerRegistrationEvent;
    class CapabilitiesGainedEvent;
    class CapabilitiesLostEvent;
    class SubscribeEvent;
    class SubscribeAckEvent;

    class EventHandler
    {
    public:
      virtual ~EventHandler() {}

      virtual void handleCancelJobAckEvent (const sdpa::events::CancelJobAckEvent *) {}
      virtual void handleCancelJobEvent (const sdpa::events::CancelJobEvent *) {}
      virtual void handleDeleteJobAckEvent (const sdpa::events::DeleteJobAckEvent *) {}
      virtual void handleDeleteJobEvent (const sdpa::events::DeleteJobEvent *) {}
      virtual void handleErrorEvent (const sdpa::events::ErrorEvent *) {}
      virtual void handleJobFailedAckEvent (const sdpa::events::JobFailedAckEvent *){}
      virtual void handleJobFailedEvent (const sdpa::events::JobFailedEvent *) {}
      virtual void handleJobFinishedAckEvent (const sdpa::events::JobFinishedAckEvent *) {}
      virtual void handleJobFinishedEvent (const sdpa::events::JobFinishedEvent *) {}
      virtual void handleJobResultsReplyEvent (const sdpa::events::JobResultsReplyEvent *) {}
      virtual void handleJobStatusReplyEvent (const sdpa::events::JobStatusReplyEvent *) {}
      virtual void handleLifeSignEvent (const sdpa::events::LifeSignEvent *) {}
      virtual void handleQueryJobStatusEvent (const sdpa::events::QueryJobStatusEvent *) {}
      virtual void handleRequestJobEvent (const sdpa::events::RequestJobEvent *) {}
      virtual void handleRetrieveJobResultsEvent (const sdpa::events::RetrieveJobResultsEvent *) {}
      virtual void handleRunJobEvent (const sdpa::events::RunJobEvent *) {}
      virtual void handleSubmitJobAckEvent (const sdpa::events::SubmitJobAckEvent *) {}
      virtual void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent *) {}
      virtual void handleWorkerRegistrationAckEvent (const sdpa::events::WorkerRegistrationAckEvent *) {}
      virtual void handleWorkerRegistrationEvent (const sdpa::events::WorkerRegistrationEvent *) {}
      virtual void handleCapabilitiesGainedEvent (const sdpa::events::CapabilitiesGainedEvent*) {}
      virtual void handleCapabilitiesLostEvent (const sdpa::events::CapabilitiesLostEvent*) {}
      virtual void handleSubscribeEvent (const sdpa::events::SubscribeEvent*) {}
      virtual void handleSubscribeAckEvent (const sdpa::events::SubscribeAckEvent*) {}
    };
  }
}

#endif
