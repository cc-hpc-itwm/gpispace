#pragma once

#include <fhgcom/header.hpp>

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
    class worker_registration_response;
    class WorkerRegistrationEvent;
    class CapabilitiesGainedEvent;
    class CapabilitiesLostEvent;
    class SubscribeEvent;
    class SubscribeAckEvent;
    class put_token;
    class put_token_response;
    class BacklogNoLongerFullEvent;
    class workflow_response;
    class workflow_response_response;

    class EventHandler
    {
    public:
      virtual ~EventHandler() = default;

      virtual void handleCancelJobAckEvent (fhg::com::p2p::address_t const&, const CancelJobAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CancelJobAck"); }
      virtual void handleCancelJobEvent (fhg::com::p2p::address_t const&, const CancelJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CancelJob"); }
      virtual void handleDeleteJobEvent (fhg::com::p2p::address_t const&, const DeleteJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DeleteJob"); }
      virtual void handleErrorEvent (fhg::com::p2p::address_t const&, const ErrorEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: ErrorEvent"); }
      virtual void handleJobFailedAckEvent (fhg::com::p2p::address_t const&, const JobFailedAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFailedAck"); }
      virtual void handleJobFailedEvent (fhg::com::p2p::address_t const&, const JobFailedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFailed"); }
      virtual void handleJobFinishedAckEvent (fhg::com::p2p::address_t const&, const JobFinishedAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFinishedAck"); }
      virtual void handleJobFinishedEvent (fhg::com::p2p::address_t const&, const JobFinishedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobFinished"); }
      virtual void handleJobResultsReplyEvent (fhg::com::p2p::address_t const&, const JobResultsReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobResultsReply"); }
      virtual void handleJobStatusReplyEvent (fhg::com::p2p::address_t const&, const JobStatusReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: JobStatusReply"); }
      virtual void handleQueryJobStatusEvent (fhg::com::p2p::address_t const&, const QueryJobStatusEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: QueryJobStatus"); }
      virtual void handleRetrieveJobResultsEvent (fhg::com::p2p::address_t const&, const RetrieveJobResultsEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: RetrieveJobResults"); }
      virtual void handleSubmitJobAckEvent (fhg::com::p2p::address_t const&, const SubmitJobAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubmitJobAck"); }
      virtual void handleSubmitJobEvent (fhg::com::p2p::address_t const&, const SubmitJobEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubmitJob"); }
      virtual void handle_worker_registration_response (fhg::com::p2p::address_t const&, const worker_registration_response*)
      { throw std::runtime_error ("UNHANDLED EVENT: WorkerRegistrationAck"); }
      virtual void handleWorkerRegistrationEvent (fhg::com::p2p::address_t const&, const WorkerRegistrationEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: WorkerRegistration"); }
      virtual void handleCapabilitiesGainedEvent (fhg::com::p2p::address_t const&, const CapabilitiesGainedEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CapabilitiesGained"); }
      virtual void handleCapabilitiesLostEvent (fhg::com::p2p::address_t const&, const CapabilitiesLostEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: CapabilitiesLost"); }
      virtual void handleSubscribeEvent (fhg::com::p2p::address_t const&, const SubscribeEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: Subscribe"); }
      virtual void handleSubscribeAckEvent (fhg::com::p2p::address_t const&, const SubscribeAckEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: SubscribeAck"); }
      virtual void handleDiscoverJobStatesEvent (fhg::com::p2p::address_t const&, const DiscoverJobStatesEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DiscoverJobStates"); }
      virtual void handleDiscoverJobStatesReplyEvent (fhg::com::p2p::address_t const&, const DiscoverJobStatesReplyEvent*)
      { throw std::runtime_error ("UNHANDLED EVENT: DiscoverJobStatesReply"); }
      virtual void handle_put_token (fhg::com::p2p::address_t const&, const put_token*)
      { throw std::runtime_error ("UNHANDLED EVENT: put_token"); }
      virtual void handle_put_token_response (fhg::com::p2p::address_t const&, const put_token_response*)
      { throw std::runtime_error ("UNHANDLED EVENT: put_token_response"); }
      virtual void handleBacklogNoLongerFullEvent (fhg::com::p2p::address_t const&, const BacklogNoLongerFullEvent*)
         { throw std::runtime_error ("UNHANDLED EVENT: BacklogNoLongerFullEvent"); }
      virtual void handle_workflow_response (fhg::com::p2p::address_t const&, const workflow_response*)
      { throw std::runtime_error ("UNHANDLED EVENT: workflow_response"); }
      virtual void handle_workflow_response_response (fhg::com::p2p::address_t const&, const workflow_response_response*)
      { throw std::runtime_error ("UNHANDLED EVENT: workflow_response_response"); }
    };
  }
}
