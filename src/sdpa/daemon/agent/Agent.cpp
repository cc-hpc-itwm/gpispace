#include <sdpa/daemon/agent/Agent.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>

#include <util-generic/join.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

namespace sdpa
{
  namespace daemon
  {
    Agent::Agent ( const std::string& name
                 , const std::string& url
                 , std::unique_ptr<boost::asio::io_service> peer_io_service
                 , boost::optional<boost::filesystem::path> const& virtual_memory_socket
                 , std::vector<name_host_port_tuple> const& masters
                 , const boost::optional<std::pair<std::string, boost::asio::io_service&>>& gui_info
                 , fhg::log::Logger& logger
                 )
      : GenericDaemon ( name
                      , url
                      , std::move (peer_io_service)
                      , virtual_memory_socket
                      , masters
                      , logger
                      , gui_info
                      , true
                      )
      , _event_handler_thread (&Agent::handle_events, this)
    {}

    void Agent::handle_job_termination (Job* const job)
    {
      auto const results
        (_scheduler.get_aggregated_results_if_all_terminated (job->id()));

      if (!results)
      {
        return;
      }

      //! \note rescheduled: never tell workflow engine or modify state!
      if (job->getStatus() == sdpa::status::PENDING)
      {
        _scheduler.releaseReservation (job->id());
        _scheduler.enqueueJob (job->id());
        request_scheduling();

        return;
      }

      //! \todo instead of ignoring sub-failures and merging error
      //! messages, just pass on results to the user
      if (job->getStatus() == sdpa::status::CANCELING)
      {
        job_canceled (job);
      }
      else
      {
        std::vector<std::string> errors;
        for (auto& result : results->individual_results)
        {
          if ( JobFSM_::s_failed const* as_failed
             = boost::get<JobFSM_::s_failed> (&result.second)
             )
          {
            errors.emplace_back (result.first + ": " + as_failed->message);
          }
        }

        if (errors.empty())
        {
          job_finished (job, results->last_success.result);
        }
        else
        {
          job_failed (job, fhg::util::join (errors.begin(), errors.end(), ", ").string());
        }
      }

      _scheduler.releaseReservation (job->id());
      request_scheduling();

      deleteJob (job->id());
    }

    void Agent::handleJobFinishedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFinishedEvent* pEvt)
    {
      child_proxy (this, source).job_finished_ack (pEvt->job_id());

      Job* const pJob
        (require_job (pEvt->job_id(), "job_finished for unknown job"));

      _scheduler.store_result
        ( _worker_manager.worker_by_address (source).get()->second
        , pJob->id()
        , JobFSM_::s_finished (pEvt->result())
        );

      handle_job_termination (pJob);
    }

    void Agent::handleJobFailedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, source).job_failed_ack (pEvt->job_id());

      Job* const pJob
        (require_job (pEvt->job_id(), "job_failed for unknown job"));

      _scheduler.store_result
        ( _worker_manager.worker_by_address (source).get()->second
        , pJob->id()
        , JobFSM_::s_failed (pEvt->error_message())
        );

      handle_job_termination (pJob);
    }

    void Agent::handleCancelJobAckEvent
      (fhg::com::p2p::address_t const& source, const events::CancelJobAckEvent* pEvt)
    {
      Job* const pJob
        (require_job (pEvt->job_id(), "cancel_job_ack for unknown job"));

      _scheduler.store_result
        ( _worker_manager.worker_by_address (source).get()->second
        , pJob->id()
        , JobFSM_::s_canceled()
        );

      handle_job_termination (pJob);
    }


    void Agent::handleCancelJobEvent
      (fhg::com::p2p::address_t const&, const events::CancelJobEvent* pEvt)
    {
      Job* const pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        throw std::runtime_error ("CancelJobEvent for unknown job");
      }

      if (pJob->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if (sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show (pJob->getStatus())
          );
      }

      if(pJob->getStatus() == sdpa::status::RUNNING)
      {
          pJob->CancelJob();
          workflowEngine()->cancel (pEvt->job_id());
      }
      else
      {
        job_canceled (pJob);

        deleteJob (pEvt->job_id());
      }
    }
  }
}
