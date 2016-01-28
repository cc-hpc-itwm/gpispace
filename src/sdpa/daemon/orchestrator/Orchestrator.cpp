#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/job_states.hpp>

namespace sdpa
{
  namespace daemon
  {
    Orchestrator::Orchestrator ( const std::string& name
                               , const std::string& url
                               , std::unique_ptr<boost::asio::io_service> peer_io_service
                               , fhg::log::Logger& logger
                               )
      : GenericDaemon ( name
                      , url
                      , std::move (peer_io_service)
                      , boost::none
                      , {}
                      , logger
                      )
      , _rpc_dispatcher
          (fhg::util::serialization::exception::serialization_functions())
      , _rpc_server (_rpc_dispatcher)
      , _event_handler_thread (&Orchestrator::handle_events, this)
    {}

    boost::asio::ip::tcp::endpoint Orchestrator::rpc_local_endpoint() const
    {
      return _rpc_server.local_endpoint();
    }

    void Orchestrator::handleJobFinishedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFinishedEvent* pEvt)
    {
      child_proxy (this, source).job_finished_ack (pEvt->job_id());

      Job* const pJob
        (require_job (pEvt->job_id(), "job_finished for unknown job"));

      job_finished (pJob, pEvt->result());

      _scheduler.releaseReservation (pEvt->job_id());
      request_scheduling();
    }

    void Orchestrator::handleJobFailedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, source).job_failed_ack (pEvt->job_id());

      Job* const pJob
        (require_job (pEvt->job_id(), "job_failed for unknown job"));

      job_failed (pJob, pEvt->error_message());

      _scheduler.releaseReservation (pEvt->job_id());
      request_scheduling();
    }

    void Orchestrator::handleCancelJobEvent
      (fhg::com::p2p::address_t const& source, const events::CancelJobEvent* pEvt)
    {
      boost::mutex::scoped_lock const _ (_scheduling_thread_mutex);

      Job* const pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        throw std::runtime_error ("CancelJobEvent for unknown job");
      }

      if(pJob->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show (pJob->getStatus())
          );
      }

      // send immediately an acknowledgment to the component that
      // requested the cancellation
      if (!isSubscriber (source))
      {
        parent_proxy (this, source).cancel_job_ack (pEvt->job_id());
      }

      sdpa::job_id_t const job_id (pEvt->job_id());
      pJob->CancelJob();

      const std::unordered_set<worker_id_t>
        workers_to_cancel (_worker_manager.workers_to_send_cancel (job_id));

      if (!workers_to_cancel.empty())
      {
        for (worker_id_t const& w : workers_to_cancel)
        {
          child_proxy ( this
                      , _worker_manager.address_by_worker (w).get()->second
                      ).cancel_job (job_id);
        }
      }
      else
      {
        job_canceled (pJob);

        _scheduler.delete_job (job_id);
        _scheduler.releaseReservation (job_id);
      }
    }

    void Orchestrator::handleCancelJobAckEvent
      (fhg::com::p2p::address_t const&, const events::CancelJobAckEvent* pEvt)
    {
      Job* const pJob
        (require_job (pEvt->job_id(), "cancel_job_ack for unknown job"));

      job_canceled (pJob);
    }

    void Orchestrator::handleDeleteJobEvent
      (fhg::com::p2p::address_t const& source, const events::DeleteJobEvent* evt)
    {
      Job* const pJob (findJob (evt->job_id()));
      if (!pJob)
      {
        throw std::runtime_error ("DeleteJobEvent for unknown job");
      }

      if(!sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot delete a job which is in a non-terminal state. "
            "Please, cancel it first!"
          );
      }

      deleteJob (evt->job_id());
      parent_proxy (this, source).delete_job_ack (evt->job_id());
    }
  }
}
