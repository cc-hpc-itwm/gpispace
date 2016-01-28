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
      , _event_handler_thread (&Orchestrator::handle_events, this)
    {}

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
  }
}
