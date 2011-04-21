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
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

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

void Orchestrator::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
	assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	SDPA_LOG_INFO("The job " << pEvt->job_id() << " has finished!");

	if (pEvt->from() != sdpa::daemon::WE)
	{
		// send a JobFinishedAckEvent back to the worker/slave
		JobFinishedAckEvent::Ptr evt(new JobFinishedAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

		// send the event to the slave
		//SDPA_LOG_INFO("Send JobFinishedAckEvent for the job " << pEvt->job_id() << " to the slave  "<<pEvt->from() );
		sendEventToSlave(evt);
	}

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished(pEvt);
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
            	ptr_workflow_engine_->finished(act_id, output);
            }

            try {
            	SDPA_LOG_DEBUG("Remove job "<<act_id<<" from the worker "<<worker_id;);
            	ptr_scheduler_->deleteWorkerJob ( worker_id, act_id );
			}
			catch(WorkerNotFoundException const &)
			{
				SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
			}
			catch(const JobNotDeletedException& ex)
			{
				SDPA_LOG_WARN( "Could not delete the job "
						 << act_id
						 << " from worker "
						 << worker_id
						 << "queues: "
						 << ex.what()
						 );
			}

			if( hasWorkflowEngine() )
			{
				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(act_id);
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

	//put the job into the state Failed
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFailed(pEvt);
	}
	catch(const JobNotFoundException &){
		SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
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
				ptr_workflow_engine_->failed(actId, output);
			}

			try {
				SDPA_LOG_DEBUG("Remove job "<<actId<<" from the worker "<<worker_id;);
				ptr_scheduler_->deleteWorkerJob(worker_id, pJob->id());
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
					ptr_job_man_->deleteJob(pEvt->job_id());
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
}

void Orchestrator::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  assert (pEvt);

  LOG(INFO, "cancelling job " << pEvt->job_id());

  try
  {
    Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());

    // change the job status to "Cancelling"
    pJob->CancelJob(pEvt);
    SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

    if(is_orchestrator() )
    {
      // send immediately an acknowledgment to the component that requested the cancellation
      CancelJobAckEvent::Ptr pCancelAckEvt
        (new CancelJobAckEvent( name()
                              , pEvt->from()
                              , pEvt->job_id()
                              , pEvt->id())
        );

      // only if the job was already submitted
      sendEventToMaster(pCancelAckEvt);
    }
  }
  catch(const JobNotFoundException &)
  {
    SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
    return;
  }

  if(pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine())
  {
    LOG( TRACE, "Propagate cancel job event downwards");
    try
    {
      sdpa::worker_id_t worker_id = findWorker(pEvt->job_id());

      SDPA_LOG_DEBUG("Send CancelJobEvent to the worker "<<worker_id);
      CancelJobEvent::Ptr pCancelEvt
        (new CancelJobEvent( name()
                           , worker_id
                           , pEvt->job_id()
                           , pEvt->reason()
                           )
        );
      sendEventToSlave(pCancelEvt);
    }
    catch(const NoWorkerFoundException& )
    {
      SDPA_LOG_WARN("The job was not assigned to any worker!");
    }
    catch(...)
    {
      SDPA_LOG_ERROR("Unexpected exception occurred!");
    }
  }
  else
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
        Job::ptr_t pJob(ptr_job_man_->findJob(pEvt->job_id()));

        // update the job status to "Cancelled"
        pJob->CancelJobAck(pEvt);
        SDPA_LOG_INFO("The job state is: "<<pJob->getStatus());
    }
    catch (std::exception const & ex)
    {
        LOG(WARN, "could not find job: " << ex.what());
        return;
    }

    // the acknowledgment does not come from WE and there is a WE
    if( pEvt->from() != sdpa::daemon::WE && hasWorkflowEngine() )
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

        // delete the worker job
        Worker::worker_id_t worker_id = pEvt->from();
        try
        {
            LOG(TRACE, "Remove job " << pEvt->job_id() << " from the worker "<<worker_id);
            ptr_scheduler_->deleteWorkerJob(worker_id, pEvt->job_id());
        }
        catch (const WorkerNotFoundException&)
        {
            SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
        }
        catch(const JobNotDeletedException& jnde)
        {
            LOG( ERROR
                            , "could not delete the job "
                            << pEvt->job_id()
                            << " from the worker "
                            << worker_id
                            << " : " << jnde.what()
            );
        }
    }
}

void Orchestrator::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* pEvt )
{
    try {
        Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
        pJob->RetrieveJobResults(pEvt, this);
    }
    catch(const JobNotFoundException&)
    {
        SDPA_LOG_INFO("The job "<<pEvt->job_id()<<" was not found by the JobManager");
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, "no such job") );
        sendEventToMaster(pErrorEvt);
    }
}

void Orchestrator::backup( std::ostream& os )
{
    try {
        //std::string strArchiveName(name()+".bkp");
        //SDPA_LOG_DEBUG("Backup the agent "<<name()<<" to file "<<strArchiveName);

        boost::archive::text_oarchive oa(os);
        oa.register_type(static_cast<JobManager*>(NULL));
        oa.register_type(static_cast<JobImpl*>(NULL));
        oa.register_type(static_cast<JobFSM*>(NULL));
        oa << ptr_job_man_;

        oa.register_type(static_cast<SchedulerOrch*>(NULL));
        oa.register_type(static_cast<SchedulerImpl*>(NULL));
        oa<<ptr_scheduler_;
    }
    catch(exception &e)
    {
        SDPA_LOG_INFO("Exception occurred: "<< e.what());
        return;
    }
}

void Orchestrator::recover( std::istream& is )
{

    try {

        boost::archive::text_iarchive ia(is);
        ia.register_type(static_cast<JobManager*>(NULL));
        ia.register_type(static_cast<JobImpl*>(NULL));
        ia.register_type(static_cast<JobFSM*>(NULL));
        // restore the schedule from the archive
        ia >> ptr_job_man_;

        //SDPA_LOG_INFO("Job manager after recovery: \n");
        //ptr_job_man_->print();

        ia.register_type(static_cast<SchedulerOrch*>(NULL));
        ia.register_type(static_cast<SchedulerImpl*>(NULL));
        ia>> ptr_scheduler_;

        //SDPA_LOG_INFO("Worker manager after recovery: \n");
        //scheduler()->print();
    }
    catch(exception &e)
    {
        SDPA_LOG_INFO("Exception occurred: "<< e.what());
    }
}
