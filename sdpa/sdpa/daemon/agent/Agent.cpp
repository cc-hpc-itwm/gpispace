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
                 , std::string kvs_host
                 , std::string kvs_port
                 , const sdpa::master_info_list_t arrMasterNames
                 , const boost::optional<std::string>& guiUrl
                 )
      : GenericDaemon (name, url, kvs_host, kvs_port, arrMasterNames, guiUrl, true)
    {}

    void Agent::handleJobFinishedEvent (const events::JobFinishedEvent* pEvt)
    {
      child_proxy (this, pEvt->from()).job_finished_ack (pEvt->job_id());

      if (scheduler().worker_manager().checkIfAbortedJobAndDelete (pEvt->from(), pEvt->job_id()))
      {
        request_scheduling();
        return;
      }

      Job* pJob (findJob (pEvt->job_id()));
      if (!hasWorkflowEngine())
      {
        pJob->JobFinished (pEvt->result());
        parent_proxy (this, pJob->owner()).job_finished
          (pEvt->job_id(), pEvt->result());
      }
      else
      {
        scheduler().workerFinished (pEvt->from(), pEvt->job_id());

        const bool bAllPartResCollected
          (scheduler().allPartialResultsCollected (pEvt->job_id()));

        if (bAllPartResCollected)
        {
          if (pJob->getStatus() == sdpa::status::CANCELING)
          {
            pJob->CancelJobAck();
            workflowEngine()->canceled (pEvt->job_id());
          }
          else if(scheduler().groupFinished (pEvt->job_id()))
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

          for (std::string worker : scheduler().workers (pJob->id()))
          {
            scheduler().worker_manager().findWorker (worker)->deleteJob (pJob->id());
          }
          scheduler().releaseReservation (pJob->id());
        }
        request_scheduling();

        if(bAllPartResCollected)
        {
          deleteJob (pEvt->job_id());
        }
      }
    }

    void Agent::handleJobFailedEvent (const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, pEvt->from()).job_failed_ack (pEvt->job_id());

      if (scheduler().worker_manager().checkIfAbortedJobAndDelete (pEvt->from(), pEvt->job_id()))
      {
        request_scheduling();
        return;
      }

      Job* pJob (findJob (pEvt->job_id()));
      if (!hasWorkflowEngine())
      {
        pJob->JobFailed (pEvt->error_message());
        parent_proxy (this, pJob->owner()).job_failed
          (pEvt->job_id(), pEvt->error_message());
      }
      else
      {
        scheduler().workerFailed (pEvt->from(), pEvt->job_id());
        bool bAllPartResCollected (scheduler().allPartialResultsCollected (pEvt->job_id()));

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

          for (std::string worker : scheduler().workers (pJob->id()))
          {
            scheduler().worker_manager().findWorker (worker)->deleteJob (pJob->id());
          }
          scheduler().releaseReservation (pJob->id());
        }
        request_scheduling();

        if (bAllPartResCollected)
        {
          deleteJob (pEvt->job_id());
        }
      }
    }

    void Agent::handleCancelJobEvent (const events::CancelJobEvent* pEvt)
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

    void Agent::handleCancelJobAckEvent (const events::CancelJobAckEvent* pEvt)
    {
      if (scheduler().worker_manager().checkIfAbortedJobAndDelete (pEvt->from(), pEvt->job_id()))
      {
        request_scheduling();
        return;
      }

      Job* pJob (findJob(pEvt->job_id()));

      if (pJob)
      {
        try
        {
          pJob->CancelJobAck();
        }
        catch (std::exception const&)
        {
          workflowEngine()->canceled (pEvt->job_id());
          throw;
        }
      }

      if (!hasWorkflowEngine())
      {
        if (!isTop())
        {
          parent_proxy (this, pJob->owner()).cancel_job_ack (pEvt->job_id());

          deleteJob (pEvt->job_id());
        }
      }
      else
      {
        LLOG (TRACE, _logger, "informing workflow engine that the activity "<< pEvt->job_id() <<" was canceled");

        scheduler().workerCanceled (pEvt->from(), pEvt->job_id());
        const bool bTaskGroupComputed
          (scheduler().allPartialResultsCollected (pEvt->job_id()));

        if (bTaskGroupComputed)
        {
          workflowEngine()->canceled (pEvt->job_id());
        }

        try
        {
          if (bTaskGroupComputed)
          {
            for (std::string worker : scheduler().workers (pJob->id()))
            {
              scheduler().worker_manager().findWorker (worker)->deleteJob (pJob->id());
            }
            scheduler().releaseReservation (pEvt->job_id());
          }
          LLOG (TRACE, _logger, "Remove job " << pEvt->job_id() << " from the worker "<<pEvt->from());
          request_scheduling();
        }
        catch (const WorkerNotFoundException&)
        {
            scheduler().delete_job (pEvt->job_id());
        }

        if (bTaskGroupComputed)
        {
          deleteJob(pEvt->job_id());
        }
      }
    }
  }
}
