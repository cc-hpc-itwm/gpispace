#ifndef SDPA_EVENT_HANDLER_HPP
#define SDPA_EVENT_HANDLER_HPP 1

#include <stdexcept>

namespace sdpa
{
  namespace events
  {
    class DiscoverJobStatesEvent;
    class DiscoverJobStatesReplyEvent;
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
    class QueryJobStatusEvent;
    class RetrieveJobResultsEvent;
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

      virtual void handleCancelJobAckEvent (const CancelJobAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CancelJobAck"); }
      virtual void handleCancelJobEvent (const CancelJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CancelJob"); }
      virtual void handleDeleteJobEvent (const DeleteJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DeleteJob"); }
      virtual void handleErrorEvent (const ErrorEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: ErrorEvent"); }
      virtual void handleJobFailedAckEvent (const JobFailedAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFailedAck"); }
      virtual void handleJobFailedEvent (const JobFailedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFailed"); }
      virtual void handleJobFinishedAckEvent (const JobFinishedAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFinishedAck"); }
      virtual void handleJobFinishedEvent (const JobFinishedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFinished"); }
      virtual void handleJobResultsReplyEvent (const JobResultsReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobResultsReply"); }
      virtual void handleJobStatusReplyEvent (const JobStatusReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobStatusReply"); }
      virtual void handleQueryJobStatusEvent (const QueryJobStatusEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: QueryJobStatus"); }
      virtual void handleRetrieveJobResultsEvent (const RetrieveJobResultsEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: RetrieveJobResults"); }
      virtual void handleSubmitJobAckEvent (const SubmitJobAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubmitJobAck"); }
      virtual void handleSubmitJobEvent (const SubmitJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubmitJob"); }
      virtual void handleWorkerRegistrationAckEvent (const WorkerRegistrationAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: WorkerRegistrationAck"); }
      virtual void handleWorkerRegistrationEvent (const WorkerRegistrationEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: WorkerRegistration"); }
      virtual void handleCapabilitiesGainedEvent (const CapabilitiesGainedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CapabilitiesGained"); }
      virtual void handleCapabilitiesLostEvent (const CapabilitiesLostEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CapabilitiesLost"); }
      virtual void handleSubscribeEvent (const SubscribeEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: Subscribe"); }
      virtual void handleSubscribeAckEvent (const SubscribeAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubscribeAck"); }
      virtual void handleDiscoverJobStatesEvent (const DiscoverJobStatesEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DiscoverJobStates"); }
      virtual void handleDiscoverJobStatesReplyEvent (const DiscoverJobStatesReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DiscoverJobStatesReply"); }
    };
  }
}

#endif
