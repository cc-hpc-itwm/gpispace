// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/agent/Agent.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>

#include <we/type/value/poke.hpp>

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

      // send a JobFinishedAckEvent back to the worker/slave
      child_proxy (this, pEvt->from()).job_finished_ack (pEvt->job_id());

      // put the job into the state Finished
      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
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
        Worker::worker_id_t worker_id = pEvt->from();
        we::layer::id_type actId = pEvt->job_id();

        // update the status of the reservation
        scheduler()->workerFinished (worker_id, actId);

        bool bAllPartResCollected = scheduler()->allPartialResultsCollected (actId);

        // if all the partial results were collected, notify the workflow
        // engine about the status of the job (either finished, or failed
        // the group is finished when all the partial results are "finished"
        if (bAllPartResCollected)
        {
          if (pJob->getStatus() == sdpa::status::CANCELING)
          {
            pJob->CancelJobAck();
            workflowEngine()->canceled (actId);
          }
          else if(scheduler()->groupFinished (actId))
          {
            pJob->JobFinished (pEvt->result());
            workflowEngine()->finished
              (actId, we::type::activity_t (pEvt->result()));
          }
          else
          {
            pJob->JobFailed
              ("One of tasks of the group failed with the actual reservation!");

            workflowEngine()->failed
              (actId, "One of tasks of the group failed with the actual reservation!");
          }
        }

        // if all partial results were collected, release the reservation
        if (bAllPartResCollected)
        {
          scheduler()->releaseReservation (pJob->id());
        }
        scheduler()->deleteWorkerJob (worker_id, pJob->id());
        request_scheduling();

        //delete it also from job_map_
        if(bAllPartResCollected)
        {
          deleteJob (pEvt->job_id());
        }
      }
    }

    void Agent::handleJobFailedEvent (const events::JobFailedEvent* pEvt)
    {
      // check if the message comes from outside/slave or from WFE
      // if it comes from a slave, one should inform WFE -> subjob
      // if it comes from WFE -> concerns the master job

      // if the event comes from the workflow engine (e.g. submission failed,
      // see the scheduler

      if (!pEvt->is_external())
      {
        failed (pEvt->job_id(), pEvt->error_message());

        return;
      }

      // send a JobFailedAckEvent back to the worker/slave
      child_proxy (this, pEvt->from()).job_finished_ack (pEvt->job_id());

      //put the job into the state Failed
      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
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
        Worker::worker_id_t worker_id = pEvt->from();

        we::layer::id_type actId = pEvt->job_id();

        // this should only be called once, therefore the state
        // machine when we switch the job from one state to another,
        // the code belonging to exactly that transition should be
        // executed. I.e. all this code should go to the FSM callback
        // routine.

        // update the status of the reservation

        bool bAllPartResCollected (false);
        scheduler()->workerFailed (worker_id, actId);
        bAllPartResCollected = scheduler()->allPartialResultsCollected (actId);

        if (bAllPartResCollected)
        {
          if (pJob->getStatus() == sdpa::status::CANCELING)
          {
            pJob->CancelJobAck();
            workflowEngine()->canceled (actId);
          }
          else
          {
            pJob->JobFailed (pEvt->error_message());
            workflowEngine()->failed (actId, pEvt->error_message());
          }

          // cancel the other jobs assigned to the workers which are
          // in the reservation list
        }

        // if all the partial results were collected, release the reservation
        if(bAllPartResCollected)
        {
          scheduler()->releaseReservation (pJob->id());
        }
        scheduler()->deleteWorkerJob (worker_id, pJob->id());
        request_scheduling();

        //delete it also from job_map_
        if (bAllPartResCollected)
        {
          deleteJob (pEvt->job_id());
        }
      }
    }

    void Agent::handleCancelJobEvent (const events::CancelJobEvent* pEvt )
    {
      Job* pJob (findJob(pEvt->job_id()));
      if (!pJob)
      {
        if (pEvt->is_external())
        {
          throw std::runtime_error ("No such job found");
        }

        return;
      }

      if (pEvt->is_external())
      {
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

        // a Cancel message came from the upper level -> forward
        // cancellation request to WE
        workflowEngine()->cancel (pEvt->job_id());
        pJob->CancelJob();
      }
      else // the workflow engine issued the cancelation order for this job
      {
        boost::optional<sdpa::worker_id_t> worker_id
          (scheduler()->findSubmOrAckWorker(pEvt->job_id()));

        // change the job status to "Canceling"
        pJob->CancelJob();

        if (worker_id)
        {
          child_proxy (this, *worker_id).cancel_job (pEvt->job_id());
        }
        else
        {
          workflowEngine()->canceled (pEvt->job_id());

          // reply with an ack here
          pJob->CancelJobAck();
          ptr_scheduler_->delete_job (pEvt->job_id());

          deleteJob (pEvt->job_id());
        }
      }
    }

    void Agent::handleCancelJobAckEvent (const events::CancelJobAckEvent* pEvt)
    {
      Job* pJob (findJob(pEvt->job_id()));

      if (pJob)
      {
        try
        {
          // update the job status to "Canceled"
          pJob->CancelJobAck();
        }
        catch (std::exception const&)
        {
          workflowEngine()->canceled (pEvt->job_id());
          throw;
        }
      }

      // the acknowledgment comes from WE or from a slave and there is no WE
      if (!pEvt->is_external() || !hasWorkflowEngine())
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
        we::layer::id_type actId = pEvt->job_id();
        Worker::worker_id_t worker_id = pEvt->from();

        scheduler()->workerCanceled (worker_id, actId);
        bool bTaskGroupComputed (scheduler()->allPartialResultsCollected (actId));

        if (bTaskGroupComputed)
        {
          workflowEngine()->canceled (pEvt->job_id());
        }

        try
        {
          if (bTaskGroupComputed)
          {
            scheduler()->releaseReservation (pEvt->job_id());
          }
          LLOG (TRACE, _logger, "Remove job " << pEvt->job_id() << " from the worker "<<worker_id);
          scheduler()->deleteWorkerJob (worker_id, pEvt->job_id());
          request_scheduling();
        }
        catch (const WorkerNotFoundException&)
        {
          // the job was not assigned to any worker yet -> this means that might
          // still be in the scheduler's queue
        }

        // delete the job completely from the job manager
        if (bTaskGroupComputed)
        {
          deleteJob(pEvt->job_id());
        }
      }
    }

    void Agent::handleDiscoverJobStatesEvent
      (const sdpa::events::DiscoverJobStatesEvent *pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));

      // if the event came from outside, forward it to the workflow engine
      if (pEvt->is_external())
      {
        if (!pJob)
        {
          parent_proxy (this, pEvt->from()).discover_job_states_reply
            ( pEvt->discover_id()
            , sdpa::discovery_info_t
              (pEvt->job_id(), boost::none, sdpa::discovery_info_set_t())
            );

          return;
        }

        m_map_discover_ids.insert
          ( std::make_pair ( pEvt->discover_id()
                           , job_info_t ( pEvt->from()
                                        , pEvt->job_id()
                                        , pJob->getStatus()
                                        )
                           )
          );
        workflowEngine()->discover (pEvt->discover_id(), pEvt->job_id());
      }
      else
      {
        //! Note: the layer guarantees that the job was already submitted
        workflowEngine()->discovered ( pEvt->discover_id()
                                     , sdpa::discovery_info_t
                                       ( pEvt->job_id()
                                       , pJob->getStatus()
                                       , sdpa::discovery_info_set_t()
                                       )
                                     );
      }
    }
  }
}
