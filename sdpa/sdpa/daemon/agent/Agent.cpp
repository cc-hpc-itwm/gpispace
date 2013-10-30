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

#include <sdpa/daemon/JobFSM.hpp>
#include <fhg/assert.hpp>

namespace sdpa {
  namespace daemon {

void Agent::action_configure(const StartUpEvent &se)
{
  GenericDaemon::action_configure (se);

  // should be overriden by the orchestrator, aggregator and NRE
  cfg().put("nmax_ext_job_req", 10U);
  MLOG (TRACE, "Configuring myself (agent)...");
}

void Agent::action_config_ok(const ConfigOkEvent& e)
{
  GenericDaemon::action_config_ok (e);

  // should be overriden by the orchestrator, aggregator and NRE
  DMLOG (TRACE, "Configuration (aggregator) was ok");

  cfg().print();
}

void Agent::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  DMLOG (TRACE, "Called handleJobFinished for the job " << pEvt->job_id());

  // send a JobFinishedAckEvent back to the worker/slave
  JobFinishedAckEvent::Ptr pEvtJobFinishedAckEvt(new JobFinishedAckEvent( name()
                                                                          , pEvt->from()
                                                                          , pEvt->job_id()
                                                                         ));
  // send the event to the slave
  sendEventToSlave(pEvtJobFinishedAckEvt);

  // put the job into the state Finished
  Job::ptr_t pJob;
  try {
    pJob = jobManager()->findJob(pEvt->job_id());
    pJob->JobFinished(pEvt);
  }
  catch(JobNotFoundException const &)
  {
    SDPA_LOG_WARN( "got finished message for old/unknown Job "<< pEvt->job_id());
    return;
  }

  if( !hasWorkflowEngine() )
  {
    try {
      // forward it up
      JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent( name()
                                                                  , pJob->owner()
                                                                  , pEvt->job_id()
                                                                  , pEvt->result()
                                                                ));

      // send the event to the master
      sendEventToMaster(pEvtJobFinished);
    }
    catch(QueueFull const &)
    {
      SDPA_LOG_ERROR("Failed to send to the master output stage "<<to_master_stage()->name()<<" a JobFinishedEvent");
    }
    catch(seda::StageNotFound const &)
    {
      SDPA_LOG_ERROR("Stage not found when trying to submit JobFinishedEvent");
    }
    catch(std::exception const & ex)
    {
      SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
      throw;
    }
    catch(...)
    {
      SDPA_LOG_FATAL("Unexpected exception occurred!");
      throw;
    }
  }
  else
  {
    Worker::worker_id_t worker_id = pEvt->from();
    id_type actId = pEvt->job_id();

    try {
      result_type output = pEvt->result();

      // update the status of the reservation
      scheduler()->workerFinished(worker_id, actId);

      bool bTaskGroupComputed(scheduler()->allPartialResultsCollected(actId));

      // if all the partial results were collected, notify the workflow engine
      // about the status of the job (either finished, or failed
      // the group is finished when all the partial results are "finished"
      if(bTaskGroupComputed) {
          DLOG(TRACE, "Inform WE that the activity "<<actId<<" finished");
          if(scheduler()->groupFinished(actId))
            workflowEngine()->finished(actId, output);
          else
            workflowEngine()->failed(actId, output, sdpa::events::ErrorEvent::SDPA_EUNKNOWN, "One of tasks of the group failed with the actual reservation!");
      }

      try {
          DLOG(TRACE, "Remove the job "<<actId<<" from the worker "<<worker_id);
          scheduler()->deleteWorkerJob( worker_id, pJob->id() );

          // if all partial results were collected, release the reservation
          if(bTaskGroupComputed) {
              scheduler()->releaseReservation(pJob->id());
          }
      }
      catch(WorkerNotFoundException const &)
      {
        DMLOG (TRACE, "Worker "<<worker_id<<" not found!");
        throw;
      }
      catch(const JobNotDeletedException&)
      {
        SDPA_LOG_ERROR("Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
      }

      try {
        //delete it also from job_map_
        if(bTaskGroupComputed) {
           DLOG(TRACE, "Remove the job "<<pEvt->job_id()<<" from the JobManager");
           jobManager()->deleteJob(pEvt->job_id());
        }
      }
      catch(JobNotDeletedException const &)
      {
          SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
          throw;
      }
    }
    catch(std::exception const & ex)
    {
        SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
    }
    catch(...)
    {
      SDPA_LOG_FATAL("Unexpected exception occurred!");
      throw;
    }
  }
}

bool Agent::finished(const id_type& wfid, const result_type & result)
{
  //put the job into the state Finished
  JobId id(wfid);
  DMLOG ( TRACE,
        "The workflow engine has notified the agent "<<name()<<" that the job "<<id.str()<<" finished!"
        );

  Job::ptr_t pJob;
  try {
    pJob = jobManager()->findJob(id);
  }
  catch(JobNotFoundException const &)
  {
    SDPA_LOG_WARN( "got finished message for old/unknown Job "<<id.str());
    return false;
  }

  try {
    // forward it up
    JobFinishedEvent::Ptr pEvtJobFinished
                  (new JobFinishedEvent( name()
                                       , pJob->owner()
                                       , id
                                       , result
                                       )
                  );

    pJob->JobFinished(pEvtJobFinished.get());

    if(!isSubscriber(pJob->owner()))
    {
      DMLOG (TRACE, "Post a JobFinished event to the master "<<pJob->owner());
      sendEventToMaster(pEvtJobFinished);
    }

    {
      std::list<std::string> workers; workers.push_back (name());
      const we::mgmt::type::activity_t act (pJob->description());
      const sdpa::daemon::NotificationEvent evt
        ( workers
        , pJob->id().str()
        , NotificationEvent::STATE_FINISHED
        , act
        );

      gui_service()->notify (evt);
    }

    BOOST_FOREACH(const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
    {
      if(subscribedFor(pair_subscr_joblist.first, id))
      {
        sdpa::events::SDPAEvent::Ptr ptrEvt( new JobFinishedEvent(*pEvtJobFinished) );
        ptrEvt->from() = name();
        ptrEvt->to()   = pair_subscr_joblist.first;
        sendEventToMaster(ptrEvt);
      }
    }
  }
  catch(QueueFull const &)
  {
    SDPA_LOG_ERROR("Failed to send to the master output stage "<<to_master_stage()->name()<<" a JobFinishedEvent");
    return false;
  }
  catch(seda::StageNotFound const &)
  {
    SDPA_LOG_ERROR("Stage not found when trying to submit JobFinishedEvent");
    return false;
  }
  catch(std::exception const & ex)
  {
    SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
    return false;
  }
  catch(...)
  {
    SDPA_LOG_FATAL("Unexpected exception occurred!");
    return false;
  }

  return true;
}

void Agent::handleJobFailedEvent(const JobFailedEvent* pEvt)
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  DMLOG (TRACE, "handleJobFailed(" << pEvt->job_id() << ")");

  // if the event comes from the workflow engine (e.g. submission failed,
  // see the scheduler

  if( pEvt->from() == sdpa::daemon::WE )
  {
    failed( pEvt->job_id()
            , pEvt->result()
            , pEvt->error_code()
            , pEvt->error_message());

    return;
  }

  // send a JobFailedAckEvent back to the worker/slave
  JobFailedAckEvent::Ptr pEvtJobFailedAckEvt(new JobFailedAckEvent( name()
                                                                    , pEvt->from()
                                                                    , pEvt->job_id() ));
  // send the event to the slave
  sendEventToSlave(pEvtJobFailedAckEvt);

  //put the job into the state Failed
  Job::ptr_t pJob;
  try {
    pJob = jobManager()->findJob(pEvt->job_id());
    pJob->JobFailed(pEvt);
  }
  catch(JobNotFoundException const &)
  {
    SDPA_LOG_WARN( "got failed message for old/unknown Job "<< pEvt->job_id());
    return;
  }

  if( !hasWorkflowEngine() )
  {
      try {
      // forward it up
      JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent(  name()
                                                              , pJob->owner()
                                                              , pEvt->job_id()
                                                              , pEvt->result() ));

      pEvtJobFailed->error_code() = pEvt->error_code();
      pEvtJobFailed->error_message() = pEvt->error_message();

      // send the event to the master
      sendEventToMaster(pEvtJobFailed);
    }
    catch(QueueFull const &)
    {
      SDPA_LOG_ERROR("Failed to send to the master output stage "<<to_master_stage()->name()<<" a JobFailedEvent");
    }
    catch(seda::StageNotFound const &)
    {
      SDPA_LOG_ERROR("Stage not found when trying to submit JobFailedEvent");
    }
    catch(std::exception const & ex)
    {
      SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
      throw ex;
    }
    catch(...)
    {
      SDPA_LOG_FATAL("Unexpected exception occurred!");
      throw;
    }
  }
  else
  {
    Worker::worker_id_t worker_id = pEvt->from();

    try {
      id_type actId = pEvt->job_id();

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
          workflowEngine()->failed( actId
                                    , pEvt->result()
                                    , pEvt->error_code()
                                    , pEvt->error_message()
                                  );

          // cancel the other jobs assigned to the workers which are
          // in the reservation list
      }

      try {
        DMLOG(TRACE, "Remove the job "<<actId<<" from the worker "<<worker_id);
        scheduler()->deleteWorkerJob( worker_id, pJob->id() );

        // if all partial results were collected, release the reservation
        if(bTaskGroupComputed) {
            scheduler()->releaseReservation(pJob->id());
        }
      }
      catch(WorkerNotFoundException const &)
      {
        DMLOG (TRACE, "Worker "<<worker_id<<" not found!");
        throw;
      }
      catch(const JobNotDeletedException&)
      {
        SDPA_LOG_ERROR("Could not delete the job "<<pJob->id()<<" from the worker "<<worker_id<<"'s queues ...");
      }

      if( hasWorkflowEngine() )
      {
        try {
          //delete it also from job_map_
          DMLOG(TRACE, "Remove the job "<<pEvt->job_id()<<" from the JobManager");
          if(bTaskGroupComputed) {
              jobManager()->deleteJob(pEvt->job_id());
          }
        }
        catch(JobNotDeletedException const &ex)
        {
          SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
          throw ex;
        }
      }
    }
    catch(std::exception const & ex)
    {
      SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
      throw ex;
    }
    catch(...)
    {
      SDPA_LOG_FATAL("Unexpected exception occurred!");
      throw;
    }
  }
}

bool Agent::failed( const id_type& wfid
                  , const result_type & result
                  , int error_code
                  , std::string const & reason
                  )
{
  JobId id(wfid);
  DMLOG ( TRACE, "The workflow engine has notified the agent "<<name()<<" that the job "<<id.str()<<" failed!"
        );
  //put the job into the state Failed

  Job::ptr_t pJob;
  try {
    pJob = jobManager()->findJob(id);
  }
  catch(JobNotFoundException const &)
  {
    SDPA_LOG_WARN( "got failed message for old/unknown Job "<<id.str());
    return false;
  }

  try {
    // forward it up
    JobFailedEvent::Ptr pEvtJobFailed(  new JobFailedEvent( name()
                                        , pJob->owner()
                                        , id
                                        , result ));

    pEvtJobFailed->error_code() = error_code;
    pEvtJobFailed->error_message() = reason;

    // send the event to the master
    pJob->JobFailed(pEvtJobFailed.get());

    if(!isSubscriber(pJob->owner()))
      sendEventToMaster(pEvtJobFailed);

    {
      std::list<std::string> workers; workers.push_back (name());
      const we::mgmt::type::activity_t act (pJob->description());
      const sdpa::daemon::NotificationEvent evt
        ( workers
        , pJob->id().str()
        , NotificationEvent::STATE_FINISHED
        , act
        );

      gui_service()->notify (evt);
    }

    BOOST_FOREACH( const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
    {
      if(subscribedFor(pair_subscr_joblist.first, id))
      {
        JobFailedEvent::Ptr ptrEvt( new JobFailedEvent(*pEvtJobFailed) );
        ptrEvt->from() = name();
        ptrEvt->to() = pair_subscr_joblist.first;
        ptrEvt->error_code() = error_code;
        ptrEvt->error_message() = reason;

        sendEventToMaster(ptrEvt);
      }
    }
  }
  catch(QueueFull const &)
  {
    SDPA_LOG_ERROR("Failed to send to the master output stage "<<to_master_stage()->name()<<" a JobFailedEvent");
    return false;
  }
  catch(seda::StageNotFound const &)
  {
    SDPA_LOG_ERROR("Stage not found when trying to submit JobFailedEvent");
    return false;
  }
  catch(std::exception const & ex)
  {
    SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
    return false;
  }
  catch(...)
  {
    SDPA_LOG_FATAL("Unexpected exception occurred!");
    return false;
  }

  return true;
}

void Agent::cancelPendingJob (const sdpa::events::CancelJobEvent& evt)
{
  if(hasWorkflowEngine())
    workflowEngine()->cancelled(evt.job_id ());

  try
  {
    sdpa::job_id_t jobId = evt.job_id();
    Job::ptr_t pJob(ptr_job_man_->findJob(jobId));

    DMLOG (TRACE, "Cancelling the pending job "<<jobId<<" ... ");

    sdpa::events::CancelJobEvent cae;
    pJob->CancelJob(&cae);
    ptr_scheduler_->delete_job (jobId);

    try
    {
      if(!isTop())
        jobManager()->deleteJob(jobId);
    }
    catch (std::exception const & ex)
    {
      SDPA_LOG_WARN( "the workflow engine could not cancel the jobId "<<jobId<<"! Reason: "<< ex.what());

      if(!isTop())
      {
        SDPA_LOG_WARN("Unexpected error occurred when trying to delete the cancelled jobId "<<jobId<<"!");
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent( name()
                                                  , evt.from()
                                                  , ErrorEvent::SDPA_EUNKNOWN
                                                  , ex.what()));

        sendEventToMaster(pErrorEvt);
      }
    }
  }
  catch(const JobNotFoundException &ex1)
  {
    SDPA_LOG_WARN( "The job "<< evt.job_id() << "could not be cancelled! Exception occurred: "<<ex1.what());
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
      sendEventToMaster(ptrEvt);

      DMLOG (TRACE, "Send an event of type "<<ptrEvt->str()<<" to the subscriber "<<pair_subscr_joblist.first<<" (related to the job "<<jobId<<")");
      break;
    }
  }
}

void Agent::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  Job::ptr_t pJob;

  try
  {
    pJob = ptr_job_man_->findJob(pEvt->job_id());
     if( isTop() )
    {
      // send immediately an acknowledgment to the component that requested the cancellation
      CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pJob->owner(), pEvt->job_id()));

      if(!isSubscriber(pJob->owner()))
        sendEventToMaster(pCancelAckEvt);

      notifySubscribers(pCancelAckEvt);
    }
  }
  catch(const JobNotFoundException &)
  {
    DMLOG (TRACE, "Job "<<pEvt->job_id()<<" not found!");

    if (pEvt->from () == sdpa::daemon::WE)
    {
      workflowEngine()->cancelled (pEvt->job_id ());
    }

    return;
  }

  if(pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine())
  {
    try
    {
      sdpa::worker_id_t worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

      SDPA_LOG_DEBUG("Tell the worker "<<worker_id<<" to cancel the job "<<pEvt->job_id());
      CancelJobEvent::Ptr pCancelEvt( new CancelJobEvent( name()
                                                          , worker_id
                                                          , pEvt->job_id()
                                                          , pEvt->reason() ) );
      sendEventToSlave(pCancelEvt);

      // change the job status to "Cancelling"
      pJob->CancelJob(pEvt);
      SDPA_LOG_DEBUG("The status of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus());
    }
    catch(const NoWorkerFoundException&)
    {
      // possible situations:
      // 1) the job wasn't yet assigned to any worker
      // 1) is in the pending queue of a certain worker
      // 1) the job was submitted to an worker but was not yet acknowledged
      cancelPendingJob(*pEvt);
    }
    catch (std::exception const & ex)
    {
      SDPA_LOG_WARN( "the workflow engine could not cancel the job "<<pEvt->job_id()<<"! Reason: "<< ex.what());

      if(!isTop())
      {
        SDPA_LOG_WARN("Unexpected error occurred when trying to delete the cancelled job "<<pEvt->job_id()<<"!");
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent( name()
                                                  , pEvt->from()
                                                  , ErrorEvent::SDPA_EUNKNOWN
                                                  , ex.what()));

        sendEventToMaster(pErrorEvt);
      }
    }
  }
  else // a Cancel message came from the upper level -> forward cancellation request to WE
  {
    id_type workflowId = pEvt->job_id();
    reason_type reason("No reason");
    DMLOG (TRACE, "Cancel the workflow "<<workflowId<<". Current status is: "<<pJob->getStatus());
    cancelWorkflow(workflowId, reason);
    pJob->CancelJob(pEvt);
    DMLOG (TRACE, "The current status of the workflow "<<workflowId<<" is: "<<pJob->getStatus());
  }
}

void Agent::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
  assert (pEvt);

  DLOG(TRACE, "handleCancelJobAck(" << pEvt->job_id() << ")");

  try
  {
    Job::ptr_t pJob(jobManager()->findJob(pEvt->job_id()));

    // update the job status to "Cancelled"
    pJob->CancelJobAck(pEvt);
    SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());
  }
  catch (std::exception const & ex)
  {
    LOG(WARN, "could not find job: " << ex.what());

    workflowEngine()->cancelled (pEvt->job_id ());

    return;
  }

  // the acknowledgment comes from WE or from a slave and there is no WE
  if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() )
  {
    // just send an acknowledgment to the master
    // send an acknowledgment to the component that requested the cancellation
    if(!isTop())
    {
      CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id() ));
      // only if the job was already submitted
      sendEventToMaster(pCancelAckEvt);

      try
      {
        jobManager()->deleteJob(pEvt->job_id());
      }
      catch(const JobNotDeletedException&)
      {
        LOG( WARN, "the JobManager could not delete the job: "<< pEvt->job_id());
      }
    }
  }
  else // acknowledgment comes from a worker -> inform WE that the activity was canceled
  {
    LOG( TRACE, "informing workflow engine that the activity "<< pEvt->job_id() <<" was cancelled");
    id_type actId = pEvt->job_id();
    Worker::worker_id_t worker_id = pEvt->from();

    scheduler()->workerCanceled(worker_id, actId);
    bool bTaskGroupComputed(scheduler()->allPartialResultsCollected(actId));

    try {
        if(bTaskGroupComputed) {
            workflowEngine()->cancelled(pEvt->job_id());
        }
    }
    catch (std::exception const & ex)
    {
      LOG(ERROR, "could not cancel job on the workflow engine: " << ex.what());
    }

    try {
      LOG(TRACE, "Remove job " << pEvt->job_id() << " from the worker "<<worker_id);
      scheduler()->deleteWorkerJob(worker_id, pEvt->job_id());

      if(bTaskGroupComputed) {
          scheduler()->releaseReservation(pEvt->job_id());
       }
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
          jobManager()->deleteJob(pEvt->job_id());
        }
    }
    catch(const JobNotDeletedException&)
    {
      LOG( WARN, "the JobManager could not delete the job: "<< pEvt->job_id());
    }
  }
}

void Agent::backup( std::ostream& ofs )
{
  try {
    //std::string strArchiveName(name()+".bkp");
    //SDPA_LOG_DEBUG("Backup the agent "<<name()<<" to file "<<strArchiveName);

    boost::archive::text_oarchive oa(ofs);
    oa.register_type(static_cast<JobManager*>(NULL));
    oa.register_type(static_cast<JobImpl*>(NULL));
    oa.register_type(static_cast<JobFSM*>(NULL));
    backupJobManager(oa);

    oa.register_type(static_cast<AgentScheduler*>(NULL));
    oa.register_type(static_cast<SchedulerImpl*>(NULL));
    backupScheduler(oa);

    /*oa.register_type(static_cast<T*>(NULL));
    oa << ptr_workflow_engine_;*/
    oa << boost::serialization::make_nvp("url_", m_arrMasterInfo);
  }
  catch(exception &e) {
    cout <<"Exception occurred: "<< e.what() << endl;
  }
}

void Agent::recover( std::istream& ifs )
{
  try {
    boost::archive::text_iarchive ia(ifs);
    ia.register_type(static_cast<JobManager*>(NULL));
    ia.register_type(static_cast<JobImpl*>(NULL));
    ia.register_type(static_cast<JobFSM*>(NULL));
    recoverJobManager(ia);

    ia.register_type(static_cast<AgentScheduler*>(NULL));
    ia.register_type(static_cast<SchedulerImpl*>(NULL));
    recoverScheduler(ia);

    // should ignore the workflow engine,
    // since it is not always possible to recover it

    /*ia.register_type(static_cast<T*>(NULL));
    ia >> ptr_workflow_engine_;*/
    ia >> boost::serialization::make_nvp("url_", m_arrMasterInfo);
    SDPA_LOG_INFO("The list of recoverd masters is: ");
  }
  catch(exception &e) {
    cout <<"Exception occurred: " << e.what() << endl;
  }
}
}}
