/*
 * =====================================================================================
 *
 *       Filename:  NRE.cpp
 *
 *    Description:  Contains the NRE class
 *
 *        Version:  1.0
 *        Created:  2009
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

// Implementation
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

template <typename U>
void NRE<U>::action_configure(const StartUpEvent &se)
{
    GenericDaemon::action_configure (se);
    SDPA_LOG_INFO("Configuring myeself (nre)...");

    // should set/update this dynamically, as a function of number of workers and their
    // processing capacities

    cfg().put("nmax_ext_job_req", 2U);
    cfg().put("polling interval", 1000 * 1000); //1s
}

template <typename U>
void NRE<U>::action_config_ok(const ConfigOkEvent& e)
{
    GenericDaemon::action_config_ok (e);

    // should be overriden by the orchestrator, aggregator and NRE
    SDPA_LOG_INFO("Configuration (nre) was ok");

    cfg().print();
}

template <typename U>
void NRE<U>::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
    assert (pEvt);

    // check if the message comes from outside/slave or from WFE
    // if it comes from a slave, one should inform WFE -> subjob
    // if it comes from WFE -> concerns the master job

    DLOG(TRACE, "handleJobFinishedEvent(" << pEvt->job_id() << ")");

    //put the job into the state Finished
    Job::ptr_t pJob;
    try {
        pJob = ptr_job_man_->findJob(pEvt->job_id());
        pJob->JobFinished(pEvt);
    }
    catch(JobNotFoundException const &)
    {
        SDPA_LOG_ERROR("Job "<<pEvt->job_id()<<" not found!");
        return;
    }

    if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
    {
        try {
            // forward it up
            JobFinishedEvent::Ptr pEvtJobFinished( new JobFinishedEvent( name(),
                                                                         pJob->owner()/*master()*/,
                                                                         pEvt->job_id(),
                                                                         pEvt->result()) );
            // send the event to the master
            sendEventToMaster(pEvtJobFinished);
            // delete it from the map when you receive a JobFinishedAckEvent!
        }
        catch(QueueFull const &)
        {
            SDPA_LOG_ERROR("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFinishedEvent");
        }
        catch(seda::StageNotFound const &)
        {
            SDPA_LOG_ERROR("Stage not found when trying to submit JobFinishedEvent");
            throw;
        }
        catch(std::exception const & ex)
        {
            SDPA_LOG_ERROR("Exception during stage->send: " << ex.what());
            throw;
        }
        catch(...)
        {
            SDPA_LOG_ERROR("Unknown exception during stage->send!");
            throw;
        }
    }
    else
    {
        LOG(ERROR, "got JobFinished event from suspicious source: " << pEvt->from());
    }
}

template <typename U>
void NRE<U>::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  DLOG(TRACE, "handleJobFailedEvent(" << pEvt->job_id() << ")");

  //put the job into the state Finished
  Job::ptr_t pJob;
  try {
      pJob = ptr_job_man_->findJob(pEvt->job_id());
      pJob->JobFailed(pEvt);
  }
  catch(JobNotFoundException const &)
  {
      SDPA_LOG_ERROR("Job "<<pEvt->job_id()<<" not found!");
      return;
  }

  if( pEvt->from() == sdpa::daemon::WE ||  !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
  {
      // the message comes from GWES, this is a local job
      // if I'm not the orchestrator
      //send JobFinished event to the master if daemon == aggregator || NRE

      try {
          // forward it up
          JobFailedEvent::Ptr pEvtJobFailedEvent(new JobFailedEvent(  name(),
                                                                      pJob->owner(),
                                                                      pEvt->job_id(),
                                                                      pEvt->result()));

          // send the event to the master
          sendEventToMaster(pEvtJobFailedEvent);
          // delete it from the map when you receive a JobFaileddAckEvent!
      }
      catch(QueueFull const &)
      {
          SDPA_LOG_WARN("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFailedEvent");
      }
      catch(seda::StageNotFound const &)
      {
          SDPA_LOG_FATAL("Stage not found when trying to send JobFailedEvent");
          throw;
      }
      catch(std::exception const & ex)
      {
          SDPA_LOG_FATAL("Exception during stage->send: " << ex.what());
          throw;
      }
      catch(...)
      {
          SDPA_LOG_FATAL("Unknown exception during stage->send!");
          throw;
      }
  }
}

template <typename U>
void  NRE<U>::cancelNotRunning(sdpa::job_id_t const & job)
{
  try
  {
    Job::ptr_t pJob(ptr_job_man_->findJob(job));

    LOG(INFO, "Update the job status to \"Cancelled\"");
    sdpa::events::CancelJobAckEvent cae;
    pJob->CancelJobAck(&cae);
    ptr_scheduler_->delete_job (job);

    try
    {
       	if(hasWorkflowEngine())
       	{
       		LOG(INFO, "Inform the workflow engine that the activity "<<job.str()<<" was cancelled!");
   			workflowEngine()->cancelled(job);
       		ptr_job_man_->deleteJob(job);
       	}
    }
    catch (std::exception const & ex)
    {
        LOG(WARN, "could not cancel job on the workflow engine: " << ex.what());
    }
  }
  catch(const JobNotFoundException &)
  {
      LOG(WARN, "job_cancelled(" << job << ") failed: no such job");
  }
}

template <typename U>
void  NRE<U>::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
    assert (pEvt);

    if( pEvt->from() == sdpa::daemon::WE )
	{
		LOG(INFO, "The workflow engine requested the cancellation of the job "<<pEvt->job_id().str());
	}
	else
	{
		LOG(INFO, "The master requested the cancellation of the job "<<pEvt->job_id().str());
	}

    try
    {
        Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());

        // change the job status to "Cancelling"
        pJob->CancelJob(pEvt);
        SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

        if(is_orchestrator())
        {
            // send immediately an acknowledgment to the component that requested the cancellation
            CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

            // only if the job was already submitted
            sendEventToMaster(pCancelAckEvt);
        }
    }
    catch(const JobNotFoundException &)
    {
        SDPA_LOG_WARN("Job "<<pEvt->job_id().str()<<" not found!");
        return;
    }

    if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() )
    {
        //sdpa::worker_id_t worker_id = findWorker(pEvt->job_id());
        //check if the job is in execution and effectively kill it
        Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());

        try {
          sdpa::status_t job_status = pJob->getStatus();
          if( job_status.find("Running") != std::string::npos ) //the job is in execution
          {
              // kill effectively the job
              // call m_worker.cancelJob(pJob->id);
              // when the nre worker notifies that the job was cancelled -> call pJob->CancelAckJob
              pJob->CancelJob(pEvt);
              DLOG(TRACE, "The job state is: "<<pJob->getStatus());

          } //if not the job is in scheduler's queue
          else
          {
              cancelNotRunning(pEvt->job_id());
          }
        }
        catch(const std::exception& ex)
        {
            SDPA_LOG_ERROR("Unexpected exception occurred:"<<ex.what());
        }
    }
    else // a Cancel message came from the upper level -> forward cancellation request to WE
    {
        id_type workflowId = pEvt->job_id();
        reason_type reason("No reason");
        cancelWorkflow(workflowId, reason);
    }
}

template <typename U>
void  NRE<U>::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
    assert (pEvt);

    DLOG(TRACE, "handleCancelJobAck(" << pEvt->job_id() << ")");

    try
    {
        Job::ptr_t pJob(ptr_job_man_->findJob(pEvt->job_id()));

        // update the job status to "Cancelled"
        pJob->CancelJobAck(pEvt);
        SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());
    }
    catch (std::exception const & ex)
    {
        LOG(WARN, "could not find job: " << ex.what());
        return;
    }

    // the acknowledgment comes from WE or from a slave and there is no WE
    if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() )
    {
        // just send an acknowledgment to the master
        // send an acknowledgment to the component that requested the cancellation
        if(!is_orchestrator())
        {
            CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));
            // only if the job was already submitted
            sendEventToMaster(pCancelAckEvt);

            try
            {
                ptr_job_man_->deleteJob(pEvt->job_id());
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

        try
        {
            ptr_workflow_engine_->cancelled(pEvt->job_id());
        }
        catch (std::exception const & ex)
        {
            LOG(ERROR, "could not cancel job on the workflow engine: " << ex.what());
        }

        // delete the job completely from the job manager
        try
        {
            ptr_job_man_->deleteJob(pEvt->job_id());
        }
        catch(const JobNotDeletedException&)
        {
            LOG( WARN, "the JobManager could not delete the job: "<< pEvt->job_id());
        }
    }
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
template <typename U>
bool  NRE<U>::cancel(const id_type& activityId, const reason_type& /* reason */)
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	/*job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);*/

	return true;
}

template <typename U>
void NRE<U>::notifyActivityCreated( const id_type& id, const std::string& data )
{
  if(hasWorkflowEngine())
  {
    activity_information_t info;
    ptr_workflow_engine_->fill_in_info (id, info);
    const std::string act_name (info.name);
    notifyObservers( NotificationEvent( id
                                      , act_name
                                      , NotificationEvent::STATE_CREATED
                                      , data
                                      )
                   );
  }
}

template <typename U>
void NRE<U>::notifyActivityStarted( const id_type& id, const std::string& data )
{
  if(hasWorkflowEngine())
  {
    activity_information_t info;
    ptr_workflow_engine_->fill_in_info (id, info);
    const std::string act_name (info.name);
    notifyObservers( NotificationEvent( id
                                      , act_name
                                      , NotificationEvent::STATE_STARTED
                                      , data
                                      )
                   );
  }
}

template <typename U>
void NRE<U>::notifyActivityFinished( const id_type& id, const std::string& result )
{
  if(hasWorkflowEngine())
  {
    activity_information_t info;
    ptr_workflow_engine_->fill_in_info (id, info);
    const std::string act_name (info.name);
    notifyObservers(NotificationEvent( id
                                     , act_name
                                     , NotificationEvent::STATE_FINISHED
                                     , result
                                     )
                   );
  }
    /*
      ApplicationGuiEvent evtAppGui(0, 0, info);
		m_appGuiService.update(evtAppGui);
*/
}

template <typename U>
void NRE<U>::notifyActivityFailed( const id_type& id, const std::string& data )
{
  if(hasWorkflowEngine())
  {
    activity_information_t info;
    ptr_workflow_engine_->fill_in_info (id, info);
    const std::string act_name (info.name);
    notifyObservers( NotificationEvent( id
                                      , act_name
                                      , NotificationEvent::STATE_FAILED
                                      , data
                                      )
                   );
  }
}

template <typename U>
void NRE<U>::notifyActivityCancelled( const id_type& id, const std::string& data )
{
    if(hasWorkflowEngine())
    {
      activity_information_t info;
      ptr_workflow_engine_->fill_in_info (id, info);
      const std::string act_name (info.name);
      notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_CANCELLED) );
    }
}

template <typename U>
void NRE<U>::backup( std::ostream& ofs )
{
    try {
        //std::string strArchiveName(name()+".bkp");
        //SDPA_LOG_DEBUG("Backup the agent "<<name()<<" to file "<<strArchiveName);

        boost::archive::text_oarchive oa(ofs);
        oa.register_type(static_cast<JobManager*>(NULL));
        oa.register_type(static_cast<JobImpl*>(NULL));
        oa.register_type(static_cast<JobFSM*>(NULL));
        oa << ptr_job_man_;

        //oa.register_type(static_cast<SchedulerNRE<U>*>(NULL));
        //oa.register_type(static_cast<SchedulerImpl*>(NULL));
        //oa<<ptr_scheduler_;
    }
    catch(exception &e)
    {
        cout <<"Exception occurred: "<< e.what() << endl;
        return;
    }
}

template <typename U>
void NRE<U>::recover( std::istream& ifs )
{

    try {
        boost::archive::text_iarchive ia(ifs);
        ia.register_type(static_cast<JobManager*>(NULL));
        ia.register_type(static_cast<JobImpl*>(NULL));
        ia.register_type(static_cast<JobFSM*>(NULL));
        // restore the schedule from the archive
        ia >> ptr_job_man_;

        //ia.register_type(static_cast<SchedulerNRE<U>*>(NULL));
        //ia.register_type(static_cast<SchedulerImpl*>(NULL));
        //ia>> ptr_scheduler_;
    }
    catch(exception &e)
    {
        cout <<"Exception occurred: " << e.what() << endl;
    }
}
