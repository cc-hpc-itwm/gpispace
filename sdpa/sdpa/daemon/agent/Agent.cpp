/*
 * =====================================================================================
 *
 *       Filename:  Agent.hpp
 *
 *    Description:  Contains the Agent class
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sdpa/daemon/agent/Agent.hpp>

#include <sdpa/daemon/Job.hpp>
#include <fhg/assert.hpp>
#include <sstream>


namespace sdpa {
  namespace daemon {

Agent::Agent ( const std::string& name
             , const std::string& url
             , const sdpa::master_info_list_t arrMasterNames
             , int rank
             , const boost::optional<std::string>& guiUrl
             )
  : GenericDaemon (name, url, arrMasterNames, rank, guiUrl, true),
    SDPA_INIT_LOGGER(name)
{
  if(rank>=0)
  {
    std::ostringstream oss;
    oss<<"rank"<<rank;

    sdpa::capability_t properCpb(oss.str(), "rank", name);
    addCapability(properCpb);
  }

  ptr_scheduler_ = SchedulerBase::ptr_t (new CoallocationScheduler (this));
  ptr_scheduler_->start_threads(); //! \note: can't do in ctor: vtable not set up yet

  //! \note Can't be moved to GenericDaemon::ctor, as
  //! requestRegistrations looks at this->capabilities, which are set
  //! after GenericDaemon::ctor. They should probably be handed down
  //! to the ctor to allow moving this code there. (it is equivalent
  //! to the one in orchestrator ctor)
  if (!isTop())
  {
    lock_type lock (mtx_master_);
    BOOST_FOREACH (sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
    {
      requestRegistration (masterInfo);
    }
  }
}

void Agent::handleJobFinishedEvent(const events::JobFinishedEvent* pEvt )
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  DMLOG (TRACE, "Called handleJobFinished for the job " << pEvt->job_id());

  // send a JobFinishedAckEvent back to the worker/slave
  events::JobFinishedAckEvent::Ptr pEvtJobFinishedAckEvt(new events::JobFinishedAckEvent( name()
                                                                          , pEvt->from()
                                                                          , pEvt->job_id()
                                                                         ));
  // send the event to the slave
  sendEventToOther(pEvtJobFinishedAckEvt);

  // put the job into the state Finished
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(!pJob)
  {
      DMLOG(WARN,  "got finished message for old/unknown Job "<< pEvt->job_id());
      return;
  }

  if( !hasWorkflowEngine() )
  {
      pJob->JobFinished(pEvt);
      // forward it up
      events::JobFinishedEvent::Ptr pEvtJobFinished(new events::JobFinishedEvent( name()
                                                                  , pJob->owner()
                                                                  , pEvt->job_id()
                                                                  , pEvt->result()
                                                                ));

      // send the event to the master
      sendEventToOther(pEvtJobFinished);
  }
  else
  {
    Worker::worker_id_t worker_id = pEvt->from();
    we::mgmt::layer::id_type actId = pEvt->job_id();

      // update the status of the reservation
      scheduler()->workerFinished(worker_id, actId);

      bool bTaskGroupComputed(scheduler()->allPartialResultsCollected(actId));

      // if all the partial results were collected, notify the workflow engine
      // about the status of the job (either finished, or failed
      // the group is finished when all the partial results are "finished"
      if(bTaskGroupComputed) {
          DLOG(TRACE, "Inform WE that the activity "<<actId<<" finished");
          if(scheduler()->groupFinished(actId))
          {
            pJob->JobFinished(pEvt);
            workflowEngine()->finished
              (actId, we::mgmt::type::activity_t (pEvt->result()));
          }
          else
          {
            events::JobFailedEvent* pJobFailedEvt(new events::JobFailedEvent( name()
                                                                             , pEvt->from()
                                                                             , pEvt->job_id()
                                                                             , fhg::error::UNEXPECTED_ERROR
                                                                             , "One of tasks of the group failed with the actual reservation!"));
            pJob->JobFailed(pJobFailedEvt);
            delete pJobFailedEvt;

            workflowEngine()->failed( actId,
                                      sdpa::events::ErrorEvent::SDPA_EUNKNOWN,
                                      "One of tasks of the group failed with the actual reservation!");
          }
      }

      try {
          DLOG(TRACE, "Remove the job "<<actId<<" from the worker "<<worker_id);
          // if all partial results were collected, release the reservation
          if(bTaskGroupComputed) {
             scheduler()->releaseReservation(pJob->id());
          }
          scheduler()->deleteWorkerJob( worker_id, pJob->id() );
      }
      catch(WorkerNotFoundException const &)
      {
        DMLOG (TRACE, "Worker "<<worker_id<<" not found!");
        throw;
      }
      catch(const JobNotDeletedException&)
      {
        DMLOG(ERROR, "Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
      }

      try {
        //delete it also from job_map_
        if(bTaskGroupComputed) {
           DLOG(TRACE, "Remove the job "<<pEvt->job_id()<<" from the JobManager");
           jobManager().deleteJob(pEvt->job_id());
        }
      }
      catch(JobNotDeletedException const &)
      {
          DMLOG(ERROR, "The JobManager could not delete the job "<<pEvt->job_id());
          throw;
      }
  }
}

void Agent::finished(const we::mgmt::layer::id_type& wfid, const we::mgmt::type::activity_t & result)
{
  //put the job into the state Finished
  JobId id(wfid);
  DMLOG ( TRACE,
        "The workflow engine has notified the agent "<<name()<<" that the job "<<id.str()<<" finished!"
        );

  Job* pJob = jobManager().findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got finished message for old/unknown Job " + id.str());
  }

  // forward it up
  events::JobFinishedEvent::Ptr pEvtJobFinished
                (new events::JobFinishedEvent( name()
                                     , pJob->owner()
                                     , id
                                     , result.to_string()
                                     )
                );

  pJob->JobFinished(pEvtJobFinished.get());

  if(!isSubscriber(pJob->owner()))
  {
    DMLOG (TRACE, "Post a JobFinished event to the master "<<pJob->owner());
    sendEventToOther(pEvtJobFinished);
  }

  if (m_guiService)
  {
    std::list<std::string> workers; workers.push_back (name());
    const we::mgmt::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( workers
      , pJob->id().str()
      , NotificationEvent::STATE_FINISHED
      , act
      );

    m_guiService->notify (evt);
  }

  BOOST_FOREACH(const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    if(subscribedFor(pair_subscr_joblist.first, id))
    {
      events::SDPAEvent::Ptr ptrEvt
        ( new events::JobFinishedEvent ( name()
                               , pair_subscr_joblist.first
                               , pEvtJobFinished->job_id()
                               , pEvtJobFinished->result()
                               )
        );

      sendEventToOther(ptrEvt);
    }
  }
}

void Agent::handleJobFailedEvent(const events::JobFailedEvent* pEvt)
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  DMLOG (TRACE, "handleJobFailed(" << pEvt->job_id() << ")");

  // if the event comes from the workflow engine (e.g. submission failed,
  // see the scheduler

  if( !pEvt->is_external() )
  {
    failed( pEvt->job_id()
            , pEvt->error_code()
            , pEvt->error_message());

    return;
  }

  // send a JobFailedAckEvent back to the worker/slave
  events::JobFailedAckEvent::Ptr pEvtJobFailedAckEvt(new events::JobFailedAckEvent( name()
                                                                    , pEvt->from()
                                                                    , pEvt->job_id() ));
  // send the event to the slave
  sendEventToOther(pEvtJobFailedAckEvt);

  //put the job into the state Failed
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(!pJob)
  {
    DMLOG(WARN,  "got failed message for old/unknown Job "<< pEvt->job_id());
    return;
  }

  if( !hasWorkflowEngine() )
  {
      pJob->JobFailed(pEvt);
      // forward it up
      events::JobFailedEvent::Ptr pEvtJobFailed
        (new events::JobFailedEvent ( name()
                            , pJob->owner()
                            , pEvt->job_id()
                            , pEvt->error_code()
                            , pEvt->error_message()
                            ));

      // send the event to the master
      sendEventToOther(pEvtJobFailed);
  }
  else
  {
    Worker::worker_id_t worker_id = pEvt->from();

      we::mgmt::layer::id_type actId = pEvt->job_id();

      // this  should only  be called  once, therefore
      // the state machine when we switch the job from
      // one state  to another, the  code belonging to
      // exactly    that    transition    should    be
      // executed. I.e. all this code should go to the
      // FSM callback routine.

      // update the status of the reservation

      scheduler()->workerFailed(worker_id, actId);
      bool bTaskGroupComputed(scheduler()->allPartialResultsCollected(actId));

      if(bTaskGroupComputed) {
          pJob->JobFailed(pEvt);
          workflowEngine()->failed( actId
                                    , pEvt->error_code()
                                    , pEvt->error_message()
                                  );

          // cancel the other jobs assigned to the workers which are
          // in the reservation list
      }

      try {
        DMLOG(TRACE, "Remove the job "<<actId<<" from the worker "<<worker_id);
        // if all the partial results were collected, release the reservation
        if(bTaskGroupComputed) {
           scheduler()->releaseReservation(pJob->id());
        }
        scheduler()->deleteWorkerJob( worker_id, pJob->id() );
      }
      catch(WorkerNotFoundException const &)
      {
        DMLOG (TRACE, "Worker "<<worker_id<<" not found!");
        throw;
      }
      catch(const JobNotDeletedException&)
      {
        DMLOG(ERROR, "Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
      }

      try {
        //delete it also from job_map_
        DMLOG(TRACE, "Remove the job "<<pEvt->job_id()<<" from the JobManager");
        if(bTaskGroupComputed) {
            jobManager().deleteJob(pEvt->job_id());
        }
      }
      catch(JobNotDeletedException const &ex)
      {
        DMLOG(ERROR, "The JobManager could not delete the job "<<pEvt->job_id());
        throw ex;
      }
  }
}

void Agent::failed( const we::mgmt::layer::id_type& wfid
                  , int error_code
                  , std::string const & reason
                  )
{
  JobId id(wfid);
  DMLOG ( TRACE, "The workflow engine has notified the agent "<<name()<<" that the job "<<id.str()<<" failed!"
        );
  //put the job into the state Failed

  Job* pJob = jobManager().findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got failed message for old/unknown Job " + id.str());
  }

  // forward it up
  events::JobFailedEvent::Ptr pEvtJobFailed
    (new events::JobFailedEvent ( name()
                        , pJob->owner()
                        , id
                        , error_code
                        , reason
                        )
    );

  // send the event to the master
  pJob->JobFailed(pEvtJobFailed.get());

  if(!isSubscriber(pJob->owner()))
    sendEventToOther(pEvtJobFailed);

  if (m_guiService)
  {
    std::list<std::string> workers; workers.push_back (name());
    const we::mgmt::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( workers
      , pJob->id().str()
      , NotificationEvent::STATE_FINISHED
      , act
      );

    m_guiService->notify (evt);
  }

  BOOST_FOREACH( const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    if(subscribedFor(pair_subscr_joblist.first, id))
    {
        events::JobFailedEvent::Ptr ptrEvt
        ( new events::JobFailedEvent ( name()
                             , pair_subscr_joblist.first
                             , pEvtJobFailed->job_id()
                             , error_code
                             , reason
                             )
        );
      sendEventToOther(ptrEvt);
    }
  }
}

namespace
{
  struct on_scope_exit
  {
    on_scope_exit (boost::function<void()> what)
      : _what (what)
      , _dont (false)
    {}
    ~on_scope_exit()
    {
      if (!_dont)
      {
        _what();
      }
    }
    void dont()
    {
      _dont = true;
    }
    boost::function<void()> _what;
    bool _dont;
  };
}

void Agent::cancelPendingJob (const sdpa::events::CancelJobEvent& evt)
{
  if(hasWorkflowEngine())
    workflowEngine()->canceled(evt.job_id ());

  sdpa::job_id_t jobId = evt.job_id();
  Job* pJob(jobManager().findJob(jobId));

  if(pJob)
  {
    DMLOG (TRACE, "Canceling the pending job "<<jobId<<" ... ");

    pJob->CancelJob(&evt);
    ptr_scheduler_->delete_job (jobId);

    if(!isTop())
    {
      on_scope_exit _ ( boost::bind ( &Agent::sendEventToOther
                                    , this
                                    , events::ErrorEvent::Ptr ( new events::ErrorEvent ( name()
                                                                       , evt.from()
                                                                       , events::ErrorEvent::SDPA_EUNKNOWN
                                                                       , "Exception in Agent::cancelPendingJob"
                                                                       )
                                                      )
                                    )
                      );

      jobManager().deleteJob(jobId);

      _.dont();
    }
  }
  else
  {
    DMLOG(WARN,  "The job "<< evt.job_id() << "could not be canceled! Exception occurred: couln't find it!");
  }
}

template <typename T>
void Agent::notifySubscribers(const T& ptrEvt)
{
  sdpa::job_id_t jobId = ptrEvt->job_id();

  BOOST_FOREACH(const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    sdpa::job_id_list_t listSubscrJobs = pair_subscr_joblist.second;

    for( sdpa::job_id_list_t::iterator it = listSubscrJobs.begin(); it != listSubscrJobs.end(); it++ )
    if( *it == jobId )
    {
      ptrEvt->to() = pair_subscr_joblist.first;
      sendEventToOther(ptrEvt);

      DMLOG (TRACE, "Send an event of type "<<ptrEvt->str()<<" to the subscriber "<<pair_subscr_joblist.first<<" (related to the job "<<jobId<<")");
      break;
    }
  }
}

void Agent::handleCancelJobEvent(const events::CancelJobEvent* pEvt )
{
  Job* pJob;

  pJob = jobManager().findJob(pEvt->job_id());
  if(!pJob)
  {
      if (pEvt->is_external())
      {
        DMLOG(TRACE, "Job "<<pEvt->job_id()<<" not found!");
        sendEventToOther( events::ErrorEvent::Ptr( new events::ErrorEvent( name()
                                                          , pEvt->from()
                                                          , events::ErrorEvent::SDPA_EJOBNOTFOUND
                                                          , "No such job found" )
                                                         ));
      }

     return;
   }

  if(pJob->getStatus() == sdpa::status::CANCELED)
  {
      sendEventToOther( events::ErrorEvent::Ptr( new events::ErrorEvent( name()
                                                          , pEvt->from()
                                                          , events::ErrorEvent::SDPA_EJOBALREADYCANCELED
                                                          , "Job already canceled" )
                                               ));
      return;
  }

  if(pJob->completed())
  {
    sendEventToOther( events::ErrorEvent::Ptr( new events::ErrorEvent( name()
                                                        , pEvt->from()
                                                        , events::ErrorEvent::SDPA_EJOBTERMINATED
                                                        , "Cannot cancel an already terminated job, its current status is: "
                                                           + sdpa::status::show(pJob->getStatus()) )
                                             ));
    return;
  }


  if( isTop() )
  {
    // send immediately an acknowledgment to the component that requested the cancellation
    events::CancelJobAckEvent::Ptr pCancelAckEvt(new events::CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id()));
    sendEventToOther(pCancelAckEvt);
    if(!isSubscriber(pEvt->from()))
      sendEventToOther(pCancelAckEvt);

    notifySubscribers(pCancelAckEvt);
  }

  if(!pEvt->is_external() || !hasWorkflowEngine())
  {
    on_scope_exit _ ( boost::bind ( &Agent::sendEventToOther
                                  , this
                                  , events::ErrorEvent::Ptr ( new events::ErrorEvent ( name()
                                                                     , pEvt->from()
                                                                     , events::ErrorEvent::SDPA_EUNKNOWN
                                                                     , "Exception in Agent::handleCancelJobEvent"
                                                                     )
                                                    )
                                  )
                    );
    if (isTop()) _.dont();

    boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

    if(worker_id)
    {
      DMLOG(TRACE, "Tell the worker "<<*worker_id<<" to cancel the job "<<pEvt->job_id());
      events::CancelJobEvent::Ptr pCancelEvt( new events::CancelJobEvent( name()
                                                          , *worker_id
                                                          , pEvt->job_id()
                                                          , pEvt->reason() ) );
      sendEventToOther(pCancelEvt);

      // change the job status to "Canceling"
      pJob->CancelJob(pEvt);
      DMLOG(TRACE, "The status of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus());
    }
    else
    {
      // possible situations:
      // 1) the job wasn't yet assigned to any worker
      // 1) is in the pending queue of a certain worker
      // 1) the job was submitted to an worker but was not yet acknowledged
      cancelPendingJob(*pEvt);
    }

    _.dont();
  }
  else // a Cancel message came from the upper level -> forward cancellation request to WE
  {
    we::mgmt::layer::id_type workflowId = pEvt->job_id();
    //! \todo "No reason"?! We've got a CancelJobEvent, which has a reason.
    we::mgmt::layer::reason_type reason("No reason");
    DMLOG (TRACE, "Cancel the workflow "<<workflowId<<". Current status is: "<<sdpa::status::show(pJob->getStatus()));
    workflowEngine()->cancel(workflowId, reason);
    pJob->CancelJob(pEvt);
    DMLOG (TRACE, "The current status of the workflow "<<workflowId<<" is: "<<sdpa::status::show(pJob->getStatus()));
  }
}

void Agent::handleCancelJobAckEvent(const events::CancelJobAckEvent* pEvt)
{
  DMLOG(TRACE, "handleCancelJobAck(" << pEvt->job_id() << ")");

  Job* pJob(jobManager().findJob(pEvt->job_id()));
  {
    on_scope_exit _ ( boost::bind ( &we::mgmt::layer::canceled
                                  , workflowEngine()
                                  , pEvt->job_id()
                                  )
                    );

    if(pJob)
    {
        // update the job status to "Canceled"
        pJob->CancelJobAck(pEvt);
        DMLOG(TRACE, "The job state is: "<<status::show(pJob->getStatus()));
    }

    _.dont();
  }

  // the acknowledgment comes from WE or from a slave and there is no WE
  if( !pEvt->is_external() || !hasWorkflowEngine() )
  {
    // just send an acknowledgment to the master
    // send an acknowledgment to the component that requested the cancellation
    if(!isTop())
    {
        events::CancelJobAckEvent::Ptr pCancelAckEvt(new events::CancelJobAckEvent(name(), pJob->owner(), pEvt->job_id() ));
      // only if the job was already submitted
      sendEventToOther(pCancelAckEvt);

      try
      {
        jobManager().deleteJob(pEvt->job_id());
      }
      catch(const JobNotDeletedException&)
      {
        LOG( WARN, "the JobManager could not delete the job: "<< pEvt->job_id());
      }
    }
  }
  else // acknowledgment comes from a worker -> inform WE that the activity was canceled
  {
    LOG( TRACE, "informing workflow engine that the activity "<< pEvt->job_id() <<" was canceled");
    we::mgmt::layer::id_type actId = pEvt->job_id();
    Worker::worker_id_t worker_id = pEvt->from();

    scheduler()->workerCanceled(worker_id, actId);
    bool bTaskGroupComputed(scheduler()->allPartialResultsCollected(actId));

        if(bTaskGroupComputed) {
            workflowEngine()->canceled(pEvt->job_id());
        }

    try {

        if(bTaskGroupComputed) {
        scheduler()->releaseReservation(pEvt->job_id());
        }
        LOG(TRACE, "Remove job " << pEvt->job_id() << " from the worker "<<worker_id);
        scheduler()->deleteWorkerJob(worker_id, pEvt->job_id());
    }
    catch (const WorkerNotFoundException&)
    {
      // the job was not assigned to any worker yet -> this means that might
      // still be in the scheduler's queue
      DMLOG (TRACE, "Worker "<<worker_id<<" not found!");
    }
    catch(const JobNotDeletedException& jnde)
    {
      LOG( ERROR, "could not delete the job " << pEvt->job_id()
                                              << " from the worker "
                                              << worker_id
                                              << " : " << jnde.what()
                                              );
    }

    // delete the job completely from the job manager
    try
    {
        if(bTaskGroupComputed) {
          jobManager().deleteJob(pEvt->job_id());
        }
    }
    catch(const JobNotDeletedException&)
    {
      LOG( WARN, "the JobManager could not delete the job: "<< pEvt->job_id());
    }
  }
}

void Agent::pause(const job_id_t& jobId)
{
  Job* pJob(jobManager().findJob(jobId));
  if(pJob)
  {
      pJob->Pause(NULL);
      if(!pJob->isMasterJob())
      {
          Job* pMasterJob(jobManager().findJob(pJob->parent()));
          if(pMasterJob)
            pMasterJob->Pause(this);
      }

      return;
   }

  DMLOG (ERROR, "Couldn't mark the worker job "<<jobId<<" as STALLED. The job was not found!");
}

void Agent::resume(const job_id_t& jobId)
{
  Job* pJob(jobManager().findJob(jobId));
  if(pJob)
  {
      pJob->Resume(NULL);
      if(!pJob->isMasterJob())
      {
          Job* pMasterJob(jobManager().findJob(pJob->parent()));
          if(pMasterJob)
            pMasterJob->Resume(this);
      }

      return;
  }

  DMLOG (WARN, "Couldn't mark the worker job "<<jobId<<" as RUNNING. The job was not found!");
}

}} // end namespaces
