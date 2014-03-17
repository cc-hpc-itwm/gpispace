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
      // check if the message comes from outside/slave or from WFE
      // if it comes from a slave, one should inform WFE -> subjob
      // if it comes from WFE -> concerns the master job

      child_proxy (this, pEvt->from()).job_finished_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

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

          scheduler().releaseReservation (pJob->id());
        }
        scheduler().worker_manager().findWorker (pEvt->from())->deleteJob (pJob->id());
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

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      if (!hasWorkflowEngine())
      {
        pJob->JobFailed (pEvt->error_message());
        parent_proxy (this, pJob->owner()).job_failed
          (pEvt->job_id(), pEvt->error_message());
      }
      else
      {
        // this should only be called once, therefore the state
        // machine when we switch the job from one state to another,
        // the code belonging to exactly that transition should be
        // executed. I.e. all this code should go to the FSM callback
        // routine.


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

          // cancel the other jobs assigned to the workers which are
          // in the reservation list

          scheduler().releaseReservation (pJob->id());
        }
        scheduler().worker_manager().findWorker (pEvt->from())->deleteJob (pJob->id());
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

      workflowEngine()->cancel (pEvt->job_id());
      pJob->CancelJob();
    }

    void Agent::handleCancelJobAckEvent (const events::CancelJobAckEvent* pEvt)
    {
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

      // the acknowledgment comes from a slave and there is no WE
      if (!hasWorkflowEngine())
      {
        // just send an acknowledgment to the master
        // send an acknowledgment to the component that requested the cancellation
        if (!isTop())
        {
          // only if the job was already submitted
          parent_proxy (this, pJob->owner()).cancel_job_ack (pEvt->job_id());

          deleteJob (pEvt->job_id());
        }
      }
      else // acknowledgment comes from a worker -> inform WE that the activity was canceled
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
            scheduler().releaseReservation (pEvt->job_id());
          }
          LLOG (TRACE, _logger, "Remove job " << pEvt->job_id() << " from the worker "<<pEvt->from());
          scheduler().worker_manager().findWorker (pEvt->from())->deleteJob (pEvt->job_id());
          request_scheduling();
        }
        catch (const WorkerNotFoundException&)
        {
          // the job was not assigned to any worker yet -> this means that might
          // still be in the scheduler's queue
        }

        if (bTaskGroupComputed)
        {
          deleteJob(pEvt->job_id());
        }
      }
    }
  }
}
