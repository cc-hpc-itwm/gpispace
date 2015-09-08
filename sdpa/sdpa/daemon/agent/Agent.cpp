// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/agent/Agent.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>

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
    {}

    void Agent::handleJobFinishedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFinishedEvent* pEvt)
    {
      child_proxy (this, source).job_finished_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!hasWorkflowEngine())
      {
        pJob->JobFinished (pEvt->result());
        parent_proxy (this, pJob->owner()).job_finished
          (pEvt->job_id(), pEvt->result());
      }
      else
      {
        _scheduler.workerFinished
          (_worker_manager.worker_by_address (source).get()->second, pEvt->job_id());

        const bool bAllPartResCollected
          (_scheduler.allPartialResultsCollected (pEvt->job_id()));

        if (bAllPartResCollected)
        {
          if (pJob->getStatus() == sdpa::status::CANCELING)
          {
            pJob->CancelJobAck();
            workflowEngine()->canceled (pEvt->job_id());
          }
          else if(_scheduler.groupFinished (pEvt->job_id()))
          {
            pJob->JobFinished (pEvt->result());
            workflowEngine()->finished
              (pEvt->job_id(), we::type::activity_t (pEvt->result()));
          }
          else
          {
            pJob->JobFailed
              ("One of tasks of the group failed with the actual reservation!");

            workflowEngine()->failed
              (pEvt->job_id(), "One of tasks of the group failed with the actual reservation!");
          }

          _scheduler.releaseReservation (pJob->id());
        }
        request_scheduling();

        if(bAllPartResCollected)
        {
          if(pJob->getStatus() != sdpa::status::PENDING)
          {
            deleteJob (pEvt->job_id());
          }
          else
          {
            _scheduler.enqueueJob (pEvt->job_id());
          }
        }
      }
    }

    void Agent::handleJobFailedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, source).job_failed_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!hasWorkflowEngine())
      {
        pJob->JobFailed (pEvt->error_message());
        parent_proxy (this, pJob->owner()).job_failed
          (pEvt->job_id(), pEvt->error_message());
      }
      else
      {
        _scheduler.workerFailed
          (_worker_manager.worker_by_address (source).get()->second, pEvt->job_id());
        bool bAllPartResCollected (_scheduler.allPartialResultsCollected (pEvt->job_id()));

        if (bAllPartResCollected)
        {
          if (pJob->getStatus() == sdpa::status::CANCELING)
          {
            pJob->CancelJobAck();
            workflowEngine()->canceled (pEvt->job_id());
          }
          else
          {
            pJob->JobFailed (pEvt->error_message());
            workflowEngine()->failed (pEvt->job_id(), pEvt->error_message());
          }

          _scheduler.releaseReservation (pJob->id());
        }
        request_scheduling();

        if (bAllPartResCollected)
        {
          if(pJob->getStatus() != sdpa::status::PENDING)
          {
            deleteJob (pEvt->job_id());
          }
          else
          {
            _scheduler.enqueueJob (pEvt->job_id());
          }
        }
      }
    }

    void Agent::handleCancelJobEvent
      (fhg::com::p2p::address_t const&, const events::CancelJobEvent* pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));
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
          parent_proxy (this, pJob->owner()).cancel_job_ack (pEvt->job_id());
          deleteJob (pEvt->job_id());
      }
    }

    void Agent::handleCancelJobAckEvent
      (fhg::com::p2p::address_t const& source, const events::CancelJobAckEvent* pEvt)
    {
      Job* pJob (findJob(pEvt->job_id()));

      if (!hasWorkflowEngine())
      {
        pJob->CancelJobAck();
        if (!isTop())
        {
          parent_proxy (this, pJob->owner()).cancel_job_ack (pEvt->job_id());

          deleteJob (pEvt->job_id());
        }
      }
      else
      {
        _scheduler.workerCanceled
          (_worker_manager.worker_by_address (source).get()->second, pEvt->job_id());
        const bool bTaskGroupComputed
          (_scheduler.allPartialResultsCollected (pEvt->job_id()));

        if (bTaskGroupComputed)
        {
          if (pJob->getStatus() == sdpa::status::CANCELING)
          {
            pJob->CancelJobAck();
            workflowEngine()->canceled (pEvt->job_id());
          }

          _scheduler.releaseReservation (pEvt->job_id());
        }

        request_scheduling();

        if (bTaskGroupComputed)
        {
          if(pJob->getStatus() != sdpa::status::PENDING)
          {
            deleteJob(pEvt->job_id());
          }
          else
          {
            _scheduler.enqueueJob (pEvt->job_id());
          }
        }
      }
    }
  }
}
