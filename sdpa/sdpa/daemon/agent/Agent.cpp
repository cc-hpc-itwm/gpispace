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
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <we/type/value/poke.hpp>
#include <fhg/assert.hpp>
#include <sstream>


namespace sdpa {
  namespace daemon {

Agent::Agent ( const std::string& name
             , const std::string& url
             , std::string kvs_host
             , std::string kvs_port
             , const sdpa::master_info_list_t arrMasterNames
             , const boost::optional<std::string>& guiUrl
             )
  : GenericDaemon (name, url, kvs_host, kvs_port, arrMasterNames, guiUrl, true)
{
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
    we::layer::id_type actId = pEvt->job_id();
    bool bAllPartResCollected(false);

    try {
        // update the status of the reservation
        scheduler()->workerFinished(worker_id, actId);

        bool bAllPartResCollected = scheduler()->allPartialResultsCollected(actId);

        // if all the partial results were collected, notify the workflow engine
        // about the status of the job (either finished, or failed
        // the group is finished when all the partial results are "finished"
        if(bAllPartResCollected)
        {
            if(pJob->getStatus() == sdpa::status::CANCELING)
            {
                events::CancelJobAckEvent evtCancelAck(name(), name(), actId );
                pJob->CancelJobAck(&evtCancelAck);
                workflowEngine()->canceled(actId);
            }
            else
              if(scheduler()->groupFinished(actId))
              {
                pJob->JobFinished(pEvt);
                workflowEngine()->finished
                  (actId, we::type::activity_t (pEvt->result()));
              }
              else
              {
                events::JobFailedEvent* pJobFailedEvt(new events::JobFailedEvent( name()
                                                                                 , pEvt->from()
                                                                                 , pEvt->job_id()
                                                                                 , "One of tasks of the group failed with the actual reservation!"));
                pJob->JobFailed(pJobFailedEvt);
                delete pJobFailedEvt;

                workflowEngine()->failed( actId,
                                          "One of tasks of the group failed with the actual reservation!");
              }
        }

        // if all partial results were collected, release the reservation
        if(bAllPartResCollected) {
           scheduler()->releaseReservation(pJob->id());
        }
        scheduler()->deleteWorkerJob( worker_id, pJob->id() );
      }
      catch(WorkerNotFoundException const &)
      {
        throw;
      }
      catch(const JobNotDeletedException&)
      {
      }

      try {
        //delete it also from job_map_
        if(bAllPartResCollected) {
           jobManager().deleteJob(pEvt->job_id());
        }
      }
      catch(JobNotDeletedException const &)
      {
          throw;
      }
  }
}

void Agent::finished(const we::layer::id_type& wfid, const we::type::activity_t & result)
{
  //put the job into the state Finished
  job_id_t id(wfid);

  Job* pJob = jobManager().findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got finished message for old/unknown Job " + id);
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
    sendEventToOther(pEvtJobFinished);
  }

  if (m_guiService)
  {
    std::list<std::string> workers; workers.push_back (name());
    const we::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( workers
      , pJob->id()
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

  // if the event comes from the workflow engine (e.g. submission failed,
  // see the scheduler

  if( !pEvt->is_external() )
  {
    failed( pEvt->job_id()
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
                            , pEvt->error_message()
                            ));

      // send the event to the master
      sendEventToOther(pEvtJobFailed);
  }
  else
  {
    Worker::worker_id_t worker_id = pEvt->from();

      we::layer::id_type actId = pEvt->job_id();

      // this  should only  be called  once, therefore
      // the state machine when we switch the job from
      // one state  to another, the  code belonging to
      // exactly    that    transition    should    be
      // executed. I.e. all this code should go to the
      // FSM callback routine.

      // update the status of the reservation

      bool bAllPartResCollected(false);
      try {
          scheduler()->workerFailed(worker_id, actId);
          bAllPartResCollected=scheduler()->allPartialResultsCollected(actId);

          if(bAllPartResCollected)
          {
              if(pJob->getStatus() == sdpa::status::CANCELING)
              {
                  events::CancelJobAckEvent evtCancelAck(name(), name(), actId );
                  pJob->CancelJobAck(&evtCancelAck);
                  workflowEngine()->canceled(actId);
              }
              else
              {
                  pJob->JobFailed(pEvt);
                  workflowEngine()->failed( actId
                                            , pEvt->error_message()
                                          );
              }

              // cancel the other jobs assigned to the workers which are
              // in the reservation list
          }

          // if all the partial results were collected, release the reservation
          if(bAllPartResCollected) {
              scheduler()->releaseReservation(pJob->id());
        }
        scheduler()->deleteWorkerJob( worker_id, pJob->id() );
      }
      catch(WorkerNotFoundException const &)
      {
        throw;
      }
      catch(const JobNotDeletedException&)
      {
      }

      try {
        //delete it also from job_map_
        if(bAllPartResCollected) {
            jobManager().deleteJob(pEvt->job_id());
        }
      }
      catch(JobNotDeletedException const &ex)
      {
        throw ex;
      }
  }
}

void Agent::failed( const we::layer::id_type& wfid
                  , std::string const & reason
                  )
{
  job_id_t id(wfid);
  //put the job into the state Failed

  Job* pJob = jobManager().findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got failed message for old/unknown Job " + id);
  }

  // forward it up
  events::JobFailedEvent::Ptr pEvtJobFailed
    (new events::JobFailedEvent ( name()
                        , pJob->owner()
                        , id
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
    const we::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( workers
      , pJob->id()
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
                             , reason
                             )
        );
      sendEventToOther(ptrEvt);
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
        throw std::runtime_error ("No such job found");
      }

     return;
  }

  if (pEvt->is_external())
  {
      if(pJob->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show(pJob->getStatus())
          );
      }

      // a Cancel message came from the upper level -> forward cancellation request to WE
      workflowEngine()->cancel(pEvt->job_id());
      pJob->CancelJob(pEvt);
  }
  else // the workflow engine issued the cancelation order for this job
  {
    boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

    events::CancelJobEvent::Ptr pCancelEvt( new events::CancelJobEvent( name()
                                                                       , worker_id.get_value_or("")
                                                                       , pEvt->job_id() ) );
    // change the job status to "Canceling"
    pJob->CancelJob(pEvt);

    if(worker_id)
    {
      sendEventToOther(pCancelEvt);
    }
    else
    {
        workflowEngine()->canceled(pEvt->job_id());

        // reply with an ack here
        events::CancelJobAckEvent evtCancelAck(name(), pEvt->from(), pEvt->job_id());
        pJob->CancelJobAck(&evtCancelAck);
        ptr_scheduler_->delete_job (pEvt->job_id());

        jobManager().deleteJob(pEvt->job_id());
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

void Agent::handleCancelJobAckEvent(const events::CancelJobAckEvent* pEvt)
{
  Job* pJob(jobManager().findJob(pEvt->job_id()));
  {
    on_scope_exit _ ( boost::bind ( &we::layer::canceled
                                  , workflowEngine()
                                  , pEvt->job_id()
                                  )
                    );

    if(pJob)
    {
        // update the job status to "Canceled"
        pJob->CancelJobAck(pEvt);
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
        LLOG (WARN, _logger,  "the JobManager could not delete the job: "<< pEvt->job_id());
      }
    }
  }
  else // acknowledgment comes from a worker -> inform WE that the activity was canceled
  {
    LLOG (TRACE, _logger, "informing workflow engine that the activity "<< pEvt->job_id() <<" was canceled");
    we::layer::id_type actId = pEvt->job_id();
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
        LLOG (TRACE, _logger, "Remove job " << pEvt->job_id() << " from the worker "<<worker_id);
        scheduler()->deleteWorkerJob(worker_id, pEvt->job_id());
    }
    catch (const WorkerNotFoundException&)
    {
      // the job was not assigned to any worker yet -> this means that might
      // still be in the scheduler's queue
    }
    catch(const JobNotDeletedException& jnde)
    {
      LLOG (ERROR, _logger, "could not delete the job " << pEvt->job_id()
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
      LLOG (WARN, _logger, "the JobManager could not delete the job: "<< pEvt->job_id());
    }
  }
}

void Agent::handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt)
{
  Job* pJob = jobManager().findJob(pEvt->job_id());

   // if the event came from outside, forward it to the workflow engine
  if(pEvt->is_external())
  {
      if(!pJob)
      {
         sendEventToOther( events::DiscoverJobStatesReplyEvent::Ptr(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                             , pEvt->from()
                                                                                                             , pEvt->discover_id()
                                                                                                             , sdpa::discovery_info_t (pEvt->job_id(), boost::none, sdpa::discovery_info_set_t()))));

         return;
      }

      m_map_discover_ids.insert(std::make_pair( pEvt->discover_id(), job_info_t(pEvt->from(), pEvt->job_id(), pJob->getStatus()) ));
      workflowEngine()->discover(pEvt->discover_id(), pEvt->job_id());
  }
  else
  {
      //! Note: the layer guarantees that the job was already submitted
      workflowEngine()->discovered(pEvt->discover_id(), sdpa::discovery_info_t (pEvt->job_id(), pJob->getStatus(), sdpa::discovery_info_set_t()));
  }
}

}} // end namespaces
