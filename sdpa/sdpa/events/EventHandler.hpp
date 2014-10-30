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
    class put_token;
    class put_token_ack;

    class EventHandler
    {
    public:
      virtual ~EventHandler() = default;

      virtual void handleCancelJobAckEvent (std::string const&, const CancelJobAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CancelJobAck"); }
      virtual void handleCancelJobEvent (std::string const&, const CancelJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CancelJob"); }
      virtual void handleDeleteJobEvent (std::string const&, const DeleteJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DeleteJob"); }
      virtual void handleErrorEvent (std::string const&, const ErrorEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: ErrorEvent"); }
      virtual void handleJobFailedAckEvent (std::string const&, const JobFailedAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFailedAck"); }
      virtual void handleJobFailedEvent (std::string const&, const JobFailedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFailed"); }
      virtual void handleJobFinishedAckEvent (std::string const&, const JobFinishedAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFinishedAck"); }
      virtual void handleJobFinishedEvent (std::string const&, const JobFinishedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFinished"); }
      virtual void handleJobResultsReplyEvent (std::string const&, const JobResultsReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobResultsReply"); }
      virtual void handleJobStatusReplyEvent (std::string const&, const JobStatusReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobStatusReply"); }
      virtual void handleQueryJobStatusEvent (std::string const&, const QueryJobStatusEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: QueryJobStatus"); }
      virtual void handleRetrieveJobResultsEvent (std::string const&, const RetrieveJobResultsEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: RetrieveJobResults"); }
      virtual void handleSubmitJobAckEvent (std::string const&, const SubmitJobAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubmitJobAck"); }
      virtual void handleSubmitJobEvent (std::string const&, const SubmitJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubmitJob"); }
      virtual void handleWorkerRegistrationAckEvent (std::string const&, const WorkerRegistrationAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: WorkerRegistrationAck"); }
      virtual void handleWorkerRegistrationEvent (std::string const&, const WorkerRegistrationEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: WorkerRegistration"); }
      virtual void handleCapabilitiesGainedEvent (std::string const&, const CapabilitiesGainedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CapabilitiesGained"); }
      virtual void handleCapabilitiesLostEvent (std::string const&, const CapabilitiesLostEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CapabilitiesLost"); }
      virtual void handleSubscribeEvent (std::string const&, const SubscribeEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: Subscribe"); }
      virtual void handleSubscribeAckEvent (std::string const&, const SubscribeAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubscribeAck"); }
      virtual void handleDiscoverJobStatesEvent (std::string const&, const DiscoverJobStatesEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DiscoverJobStates"); }
      virtual void handleDiscoverJobStatesReplyEvent (std::string const&, const DiscoverJobStatesReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DiscoverJobStatesReply"); }
      virtual void handle_put_token (std::string const&, const put_token*)
      { throw std::runtime_error ("UNHANDLED EVENT: put_token"); }
      virtual void handle_put_token_ack (std::string const&, const put_token_ack*)
      { throw std::runtime_error ("UNHANDLED EVENT: put_token_ack"); }
    };
  }
}

#endif
