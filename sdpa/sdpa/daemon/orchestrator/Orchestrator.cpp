/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
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

#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

Orchestrator::~Orchestrator()
{
  SDPA_LOG_DEBUG("Orchestrator's destructor called ...");
  //daemon_stage_ = NULL;
}

void Orchestrator::action_configure(const StartUpEvent &se)
{
  GenericDaemon::action_configure (se);

  // should be overriden by the orchestrator, aggregator and NRE
  SDPA_LOG_INFO("Configuring myeself (orchestrator)...");
}

void Orchestrator::action_config_ok(const ConfigOkEvent& e)
{
  // should be overriden by the orchestrator, aggregator and NRE

  GenericDaemon::action_config_ok (e);

  SDPA_LOG_INFO("Configuration (orchestrator) was ok");

  cfg().print();
}

template <typename T>
void Orchestrator::notifySubscribers(const T& ptrEvt)
{
	BOOST_FOREACH(const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
	{
          // DLOG(TRACE, "Notify the subscriber "<<subscriber);
		ptrEvt->to() = pair_subscr_joblist.first;
		sendEventToMaster(ptrEvt);
	}
}

void Orchestrator::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
    assert (pEvt);

    // check if the message comes from outside/slave or from WFE
    // if it comes from a slave, one should inform WFE -> subjob
    // if it comes from WFE -> concerns the master job

    MLOG(INFO, "The job " << pEvt->job_id() << " has finished!");

    if (pEvt->from() != sdpa::daemon::WE)
    {
      // send a JobFinishedAckEvent back to the worker/slave
      JobFinishedAckEvent::Ptr ptrAckEvt(new JobFinishedAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

      // send ack to the slave
      SDPA_LOG_INFO("Send JobFinishedAckEvent for the job " << pEvt->job_id() << " to the slave  "<<pEvt->from() );
      sendEventToSlave(ptrAckEvt);
    }

    //put the job into the state Finished or Cancelled
    Job::ptr_t pJob;
    try {
        pJob = jobManager()->findJob(pEvt->job_id());
        pJob->JobFinished(pEvt);
        SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());
    }
    catch(JobNotFoundException const &)
    {
        SDPA_LOG_WARN( "got finished message for old/unknown Job "<< pEvt->job_id());
        return;
    }

    // It's an worker who has sent the message
    if( (pEvt->from() != sdpa::daemon::WE) )
    {
        Worker::worker_id_t worker_id = pEvt->from();
        id_type act_id = pEvt->job_id();

        try {
            result_type output = pEvt->result();

            if( hasWorkflowEngine() )
            {
                SDPA_LOG_DEBUG("Inform the workflow engine that the activity "<<act_id<<" finished");
                workflowEngine()->finished(act_id, output);
            }
            else
            {
            	JobFinishedEvent::Ptr ptrEvtJobFinished(new JobFinishedEvent(*pEvt));
            	notifySubscribers(ptrEvtJobFinished);
            }

            try {
                SDPA_LOG_DEBUG("Remove job "<<act_id<<" from the worker "<<worker_id);
                scheduler()->deleteWorkerJob ( worker_id, act_id );
            }
            catch(WorkerNotFoundException const &)
            {
                SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
            }
            catch(const JobNotDeletedException& ex)
            {
                SDPA_LOG_WARN( "Could not delete the job " << act_id
														   << " from worker "
														   << worker_id
														   << "queues: "
														   << ex.what() );
            }

            if( hasWorkflowEngine() )
            {
                try {
                    //delete it also from job_map_
                    jobManager()->deleteJob(act_id);
                }
                catch(JobNotDeletedException const &)
                {
                    SDPA_LOG_WARN("The JobManager could not delete the job "<< act_id);
                }
            }

        }catch(...) {
            SDPA_LOG_ERROR("Unexpected exception occurred!");
        }
    }
    else
    {
    	JobFinishedEvent::Ptr ptrEvtJobFinished(new JobFinishedEvent(*pEvt));
    	notifySubscribers(ptrEvtJobFinished);
    }
}

void Orchestrator::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
    assert (pEvt);

    // check if the message comes from outside/slave or from WFE
    // if it comes from a slave, one should inform WFE -> subjob
    // if it comes from WFE -> concerns the master job

    //SDPA_LOG_INFO( "handle JobFailed event (job " << pEvt->job_id() << ") received from "<<pEvt->from());

    if (pEvt->from() != sdpa::daemon::WE)
    {
        // send a JobFinishedAckEvent back to the worker/slave
        JobFailedAckEvent::Ptr evt
            (new JobFailedAckEvent ( name()
                                                       , pEvt->from()
                                                       , pEvt->job_id()
                                                       , pEvt->id()
                                                       )
            );

        // send the event to the slave
        sendEventToSlave(evt);
    }

    //put the job into the state Failed or Cancelled
    Job::ptr_t pJob;
    try {
        pJob = jobManager()->findJob(pEvt->job_id());
        pJob->JobFailed(pEvt);
        SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());
    }
    catch(const JobNotFoundException &)
    {
        SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
        // TODO: shouldn't we reply with an error here???
        return;
    }

    // It's an worker who has sent the message
    if( (pEvt->from() != sdpa::daemon::WE) )
    {
        Worker::worker_id_t worker_id = pEvt->from();
        id_type actId = pJob->id().str();

        try {
            result_type output = pEvt->result();

            if( hasWorkflowEngine() )
            {
                SDPA_LOG_DEBUG("Inform the workflow engine that the activity "<<actId<<" failed");
                workflowEngine()->failed(actId, output);
            }
            else
            {
            	JobFailedEvent::Ptr ptrEvtJobFailed(new JobFailedEvent(*pEvt));
            	notifySubscribers(ptrEvtJobFailed);
            }

            try {
                SDPA_LOG_DEBUG("Remove the job "<<actId<<" from the worker "<<worker_id<<"'s queues");
                scheduler()->deleteWorkerJob(worker_id, pJob->id());
            }
            catch(const WorkerNotFoundException&)
            {
                SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
            }
            catch(const JobNotDeletedException&)
            {
                SDPA_LOG_WARN("Could not delete the job "<<pJob->id()<<" from the "<<worker_id<<"'s queues ...");
            }

            if( hasWorkflowEngine() )
            {
                try {
                    //delete it also from job_map_
                    jobManager()->deleteJob(pEvt->job_id());
                }
                catch(const JobNotDeletedException&)
                {
                    SDPA_LOG_WARN("The JobManager could not delete the job "<<pJob->id());
                }
            }
        }
        catch(...) {
            SDPA_LOG_ERROR("Unexpected exception occurred!");
        }
    }
    else
    {
    	JobFailedEvent::Ptr ptrEvtJobFailed(new JobFailedEvent(*pEvt));
    	notifySubscribers(ptrEvtJobFailed);
    }
}


void Orchestrator::cancelNotRunning (sdpa::job_id_t const & job)
{
  try
  {
    Job::ptr_t pJob(jobManager()->findJob(job));

    // update the job status to "Cancelled" we don't have an ack
    sdpa::events::CancelJobAckEvent cae;
    pJob->CancelJobAck(&cae);
    scheduler()->delete_job (job);

    try
    {
    	if(hasWorkflowEngine())
    	{
    		workflowEngine()->cancelled(job);
    		jobManager()->deleteJob(job);
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

void Orchestrator::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  assert (pEvt);

  LOG(INFO, "cancelling job " << pEvt->job_id());

  try
  {
    Job::ptr_t pJob = jobManager()->findJob(pEvt->job_id());

    // change the job status to "Cancelling"
    pJob->CancelJob(pEvt);
    SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

    if(isTop())
    {
        // send immediately an acknowledgment to the component that requested the cancellation
        CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

        // only if the job was already submitted
        sendEventToMaster(pCancelAckEvt);
    }
  }
  catch(const JobNotFoundException &)
  {
    SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent( name()
                                            , pEvt->from()
                                            , ErrorEvent::SDPA_EJOBNOTFOUND
                                            , "no such job"
                                            )
                             );
    sendEventToMaster(pErrorEvt);
    return;
  }

  if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine())
  {
	  LOG(TRACE, "Propagate cancel job event downwards.");
	  try
	  {
		  sdpa::worker_id_t worker_id = scheduler()->findAcknowlegedWorker(pEvt->job_id());

		  SDPA_LOG_DEBUG("Send CancelJobEvent to the worker "<<worker_id);
		  CancelJobEvent::Ptr pCancelEvt( new CancelJobEvent( name()
				  	  	  	  	  	  	  	  	  	  	  	  , worker_id
				  	  	  	  	  	  	  	  	  	  	  	  , pEvt->job_id()
				  	  	  	  	  	  	  	  	  	  	  	  , pEvt->reason()
                                       	   	   	   	   	   	) );
		  sendEventToSlave(pCancelEvt);
    }
    catch(const NoWorkerFoundException&)
    {
        cancelNotRunning(pEvt->job_id());
    }
    catch(...)
    {
        SDPA_LOG_ERROR("Unexpected exception occurred!");
    }
  }
  else // a Cancel message came from the upper level -> forward cancellation request to WE
  {
      id_type workflowId = pEvt->job_id();
      reason_type reason("No reason");
      cancelWorkflow(workflowId, reason);
  }
}

void Orchestrator::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
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
        return;
    }

    // the acknowledgment comes from WE or from a slave and there is no WE
    if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() )
    {
        // just send an acknowledgment to the master
        // send an acknowledgment to the component that requested the cancellation
    	CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

        if(!isTop())
        {
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
        else
        {
        	notifySubscribers(pCancelAckEvt);
        }
    }
    else // acknowledgment comes from a worker -> inform WE that the activity was canceled
    {
        LOG( TRACE, "informing workflow engine that the activity "<< pEvt->job_id() <<" was cancelled");

        try
        {
            workflowEngine()->cancelled(pEvt->job_id());
        }
        catch (std::exception const & ex)
        {
            LOG(ERROR, "could not cancel job on the workflow engine: " << ex.what());
        }

        // delete the worker job
        Worker::worker_id_t worker_id = pEvt->from();
        try
        {
            LOG(TRACE, "Remove job " << pEvt->job_id() << " from the worker "<<worker_id);
            scheduler()->deleteWorkerJob(worker_id, pEvt->job_id());
        }
        catch (const WorkerNotFoundException&)
        {
            // the job was not assigned to any worker yet -> this means that might
            // still be in the scheduler's queue
            SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
        }
        catch(const JobNotDeletedException& jnde)
        {
            LOG( ERROR, "could not delete the job "
                        << pEvt->job_id()
                        << " from the worker "
                        << worker_id
                        << " : " << jnde.what()
            );
        }

        // delete the job completely from the job manager
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

void Orchestrator::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* pEvt )
{
    try {
        Job::ptr_t pJob = jobManager()->findJob(pEvt->job_id());
        pJob->RetrieveJobResults(pEvt, this);
    }
    catch(const JobNotFoundException&)
    {
        SDPA_LOG_INFO("The job "<<pEvt->job_id()<<" was not found by the JobManager");
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, "no such job") );
        sendEventToMaster(pErrorEvt);
    }
}

//template <typename T>
void Orchestrator::backup( std::ostream& os )
{
    try {
        //std::string strArchiveName(name()+".bkp");
        //SDPA_LOG_DEBUG("Backup the agent "<<name()<<" to file "<<strArchiveName);

        boost::archive::text_oarchive oa(os);
        oa.register_type(static_cast<JobManager*>(NULL));
        oa.register_type(static_cast<JobImpl*>(NULL));
        oa.register_type(static_cast<JobFSM*>(NULL));
        backupJobManager(oa);

        oa.register_type(static_cast<SchedulerOrch*>(NULL));
        oa.register_type(static_cast<SchedulerImpl*>(NULL));
        backupScheduler(oa);

        /*oa.register_type(static_cast<DummyWorkflowEngine*>(NULL));
        oa << ptr_workflow_engine_;*/
        oa << boost::serialization::make_nvp("url_", m_arrMasterInfo);
    }
    catch(exception &e)
    {
        SDPA_LOG_INFO("Exception occurred: "<< e.what());
        return;
    }
}

//template <typename T>
void Orchestrator::recover( std::istream& is )
{
  try {
      boost::archive::text_iarchive ia(is);
      ia.register_type(static_cast<JobManager*>(NULL));
      ia.register_type(static_cast<JobImpl*>(NULL));
      ia.register_type(static_cast<JobFSM*>(NULL));
      // restore the schedule from the archive
      recoverJobManager(ia);

      //SDPA_LOG_INFO("Job manager after recovery: \n");
      //jobManager()->print();

      ia.register_type(static_cast<SchedulerOrch*>(NULL));
      ia.register_type(static_cast<SchedulerImpl*>(NULL));
      recoverScheduler(ia);

      // should ignore the workflow engine recovery,
      // since it is not always possible to recover it

      // re-schedule the master jobs
      // for any master job in the job_map
      // if the state is running -> go back to pending
      // or simply create a new job with the same job id

      /*ia.register_type(static_cast<T*>(NULL));
      ia >> ptr_workflow_engine_;*/
      ia >> boost::serialization::make_nvp("url_", m_arrMasterInfo);
  }
  catch(exception &e)
  {
      SDPA_LOG_INFO("Exception occurred: "<< e.what());
  }
}
