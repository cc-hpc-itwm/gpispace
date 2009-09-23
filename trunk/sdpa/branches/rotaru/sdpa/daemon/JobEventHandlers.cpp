#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
//#include <sdpa/daemon/jobFSM/BSC/JobFSM.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>
#include <sstream>
#include <map>

#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/wf/types.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::wf;
using namespace sdpa::events;


void GenericDaemon::handleJobEvent(const seda::IEvent::Ptr& pEvent)
{
	// check what type of event is and call transitions
	if( CancelJobAckEvent* ptr = dynamic_cast<CancelJobAckEvent*>(pEvent.get()) )
		handleCancelJobAckEvent(ptr);
	else if( SubmitJobAckEvent* ptr = dynamic_cast<SubmitJobAckEvent*>(pEvent.get()) )
		handleSubmitJobAckEvent(ptr);
	else if( JobFinishedEvent* ptr = dynamic_cast<JobFinishedEvent*>(pEvent.get()) )
		handleJobFinishedEvent(ptr);
	else if(	JobFailedEvent* ptr = dynamic_cast<JobFailedEvent*>(pEvent.get()))
		handleJobFailedEvent(ptr);
	else if( QueryJobStatusEvent* ptr = dynamic_cast<QueryJobStatusEvent*>(pEvent.get()) )
		handleQueryJobStatusEvent(ptr);
	else if( JobFinishedAckEvent* ptr = dynamic_cast<JobFinishedAckEvent*>(pEvent.get()) )
		handleJobFinishedAckEvent(ptr);
	else if( JobFailedAckEvent* ptr = dynamic_cast<JobFailedAckEvent*>(pEvent.get()) )
		handleJobFailedAckEvent(ptr);
	else if( CancelJobEvent* ptr = dynamic_cast<CancelJobEvent*>(pEvent.get()) )
		handleCancelJobEvent(ptr);
	else if( RetrieveJobResultsEvent* ptr = dynamic_cast<RetrieveJobResultsEvent*>(pEvent.get()) )
		handleRetrieveResultsEvent(ptr);
}

void GenericDaemon::handleSubmitJobAckEvent(const SubmitJobAckEvent* pEvent)
{
	ostringstream os;
	os<<"Call 'action_request_job_ack'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());

	// a slave posted an acknowledgement for a job request
	// put the job from pending into submitted
	// call worker :: acknowledge(const sdpa::job_id_t& job_id ) = ;
	Worker::worker_id_t worker_id = pEvent->from();
	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);
		//put the job into the Running state: do this in acknowledge!
		ptrWorker->acknowledge(pEvent->job_id());
	} catch(sdpa::daemon::WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!";
		SDPA_LOG_DEBUG(os.str());
	}
}


void GenericDaemon::handleJobFinishedEvent(const JobFinishedEvent* pEvt)
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	ostringstream os;
	os<<"Call 'action_job_finished'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());

	//put the job into the state Finished
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished();
	}
	catch(sdpa::daemon::JobNotFoundException){
		os.str("");
		os<<"Job "<<pEvt->job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}

	if(pEvt->from() == pEvt->to() && name() != std::string("orchestrator") ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES, this is a local job
		// if I'm not the orchestrator
		//send JobFinished event to the master if daemon == aggregator || NRE
		try {
			// forward it up
			JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), master(), pEvt->job_id()));

			// send the event to the master
			sendEvent(output_stage_, pEvtJobFinished);

			// delete it from the map when you receive a JobFinishedAckEvent!
		}
		catch(QueueFull)
		{
			os.str("");
			os<<"Failed to send to the output stage "<<output_stage_<<" a SubmitJobEvent";
			SDPA_LOG_DEBUG(os.str());
		}
		catch(seda::StageNotFound)
		{
			os.str("");
			os<<"Stage not found when trying to submit SubmitJobEvent";
			SDPA_LOG_DEBUG(os.str());
		}
		catch(...) {
			os.str("");
			os<<"Unexpected exception occurred!";
			SDPA_LOG_DEBUG(os.str());
		}

		// no acknowledgement to be sent to GWES
	}
	else
	{
		Worker::worker_id_t worker_id = pEvt->from();
		try {
			// Should set the workflow_id here, or send it together with the workflow description
			if(ptr_Sdpa2Gwes_)
			{
				sdpa::wf::workflow_id_t wf_id = pEvt->job_id().str();
				activity_id_t activityId;
				parameter_list_t output;

				os.str("");
				os<<"Inform GWES that the workflow "<<wf_id<<" finished";
				SDPA_LOG_DEBUG(os.str());

				ptr_Sdpa2Gwes_->activityFinished(wf_id, activityId, output);

				Worker::ptr_t ptrWorker = findWorker(worker_id);
				// delete job from worker's queues

				os.str("");
				os<<"Delete the job "<<pEvt->job_id()<<" from the worke's queues!";
				SDPA_LOG_DEBUG(os.str());

				ptrWorker->delete_job(pEvt->job_id());

				// send a JobFinishedAckEvent back to the worker/slave
				//delete it also from job_map_
				JobFinishedAckEvent::Ptr pEvtJobFinishedAck(new JobFinishedAckEvent(name(), master(), pEvt->job_id()));

				// send the event to the master
				sendEvent(output_stage_, pEvtJobFinishedAck);

				//delete it also from job_map_
				ptr_job_man_->deleteJob(pEvt->job_id());
			}
			else
				SDPA_LOG_ERROR("Gwes not initialized!");

		} catch(sdpa::daemon::WorkerNotFoundException) {
			os.str("");
			os<<"Worker "<<worker_id<<" not found!";
			SDPA_LOG_DEBUG(os.str());
		}
		catch(sdpa::daemon::NoSuchWorkflowException& )
		{
			SDPA_LOG_ERROR("NoSuchWorkflowException occurred!");
		}
		catch(sdpa::daemon::NoSuchActivityException& )
		{
			SDPA_LOG_DEBUG("NoSuchActivityException occurred!");
		}
		catch(JobNotDeletedException&)
		{
			os.str("");
			os<<"The JobManager could not delete the job "<<pEvt->job_id();
			SDPA_LOG_DEBUG(os.str());
		}
		catch(...) {
			os.str("");
			os<<"Unexpected exception occurred!";
			SDPA_LOG_DEBUG(os.str());
		}
	}
}



void GenericDaemon::handleJobFailedEvent(const JobFailedEvent* ptr )
{

}

void GenericDaemon::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job
}


// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent(const JobFinishedAckEvent* pEvt)
{
   // The result was succesfully delivered, so I can delete the job from the job map
	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a JobFinishedAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(sdpa::daemon::JobNotFoundException)
	{
		os.str("");
		os<<"Job "<<pEvt->job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(JobNotDeletedException&)
	{
		os.str("");
		os<<"The JobManager could not delete the job "<<pEvt->job_id();
		SDPA_LOG_DEBUG(os.str());
	}
	catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!";
		SDPA_LOG_DEBUG(os.str());
	}
}


// respond to a worker that the JobFailedEvent was received
void GenericDaemon::handleJobFailedAckEvent(const JobFailedAckEvent* pEvt )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job
	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a JobFinishedAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(sdpa::daemon::JobNotFoundException)
	{
		os.str("");
		os<<"Job "<<pEvt->job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(JobNotDeletedException&)
	{
		os.str("");
		os<<"The JobManager could not delete the job "<<pEvt->job_id();
		SDPA_LOG_DEBUG(os.str());
	}
	catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::handleQueryJobStatusEvent(const QueryJobStatusEvent* ptr )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
		pJob->QueryJobStatus();
	}
	catch(JobNotFoundException)
	{
		ostringstream os;
		os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::handleCancelJobEvent(const CancelJobEvent* ptr )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
		pJob->CancelJob();
	}
	catch(JobNotFoundException)
	{
		ostringstream os;
		os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::handleRetrieveResultsEvent(const RetrieveJobResultsEvent* ptr )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
		pJob->RetrieveJobResults();
	}
	catch(JobNotFoundException)
	{
		ostringstream os;
		os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
		SDPA_LOG_DEBUG(os.str());
	}
}
