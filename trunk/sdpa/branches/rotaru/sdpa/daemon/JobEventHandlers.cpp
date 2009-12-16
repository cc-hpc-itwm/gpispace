/*
 * =====================================================================================
 *
 *       Filename:  JobEventHandlers.cpp
 *
 *    Description:  Implements the handling of job related events
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
#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>
#include <map>

#include <sdpa/daemon/exceptions.hpp>


using namespace std;
using namespace sdpa::daemon;
using namespace gwes;
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
	else if( JobFailedEvent* ptr = dynamic_cast<JobFailedEvent*>(pEvent.get()))
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
	os<<"Call 'handleSubmitJobAckEvent'";
	SDPA_LOG_DEBUG(os.str());

	acknowledge(pEvent->id());

	// a slave posted an acknowledgement for a job request
	// put the job from pending into submitted
	// call worker :: acknowledge(const sdpa::job_id_t& job_id ) = ;
	Worker::worker_id_t worker_id = pEvent->from();
	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);
		//put the job into the Running state: do this in acknowledge!
		ptrWorker->acknowledge(pEvent->job_id());

	} catch(WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	ostringstream os;
	SDPA_LOG_DEBUG("Call 'handleJobFinishedEvent'");

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished(pEvt);
	}
	catch(JobNotFoundException){
		SDPA_LOG_DEBUG("Job "<<pEvt->job_id()<<" not found!");
	}

	if(pEvt->from() == sdpa::daemon::GWES ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES, this is a local job
		// if I'm not the orchestrator
		//send JobFinished event to the master if daemon == aggregator || NRE

		if( name() != ORCHESTRATOR ) // if I'm not an orchestrator forward the message up
		{
			try {
				// forward it up
				JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

				// send the event to the master
				sendEventToMaster(pEvtJobFinished, MSG_RETRY_CNT);
				// delete it from the map when you receive a JobFaileddAckEvent!
			}
			catch(QueueFull)
			{
				SDPA_LOG_DEBUG("Failed to send to the ,aster output stage "<<ptr_to_master_stage_->name()<<" a SubmitJobEvent");
			}
			catch(seda::StageNotFound)
			{
				SDPA_LOG_DEBUG("Stage not found when trying to submit SubmitJobEvent");
			}
			catch(...) {
				SDPA_LOG_DEBUG("Unexpected exception occurred!");
			}
		}
		else
			SDPA_LOG_DEBUG("Don't notify the user! He will periodically query for the job status!");

		// no acknowledgement to be sent to GWES
	}
	else //event sent by a worker
	{
		Worker::worker_id_t worker_id = pEvt->from();

		if( name() != NRE )
		{
			// send a JobFinishedAckEvent back to the worker/slave
			JobFinishedAckEvent::Ptr pEvtJobFinishedAckEvt(new JobFinishedAckEvent(name(), worker_id, pEvt->job_id(), pEvt->id()));
			sendEventToSlave(pEvtJobFinishedAckEvt);
		}

		try {
			// Should set the workflow_id here, or send it together with the workflow description
			if(ptr_Sdpa2Gwes_)
			{
				activity_id_t actId = pJob->id().str();
				workflow_id_t wfId  = pJob->parent().str();

				parameter_list_t output;

				SDPA_LOG_DEBUG("Inform GWES that the activity "<<actId<<" finished");

				try {
					ptr_Sdpa2Gwes_->activityFinished(wfId, actId, output);
				}
				catch(gwes::Gwes2Sdpa::NoSuchActivity& )
				{
					SDPA_LOG_ERROR("NoSuchActivityException occurred!");
				}

				try {
					Worker::ptr_t ptrWorker = findWorker(worker_id);
					// delete job from the worker's queues

					SDPA_LOG_DEBUG("Delete the job "<<pEvt->job_id()<<" from the worker's queues!");
					ptrWorker->delete_job(pEvt->job_id());
				}
				catch(WorkerNotFoundException)
				{
					SDPA_LOG_DEBUG("Worker "<<worker_id<<" not found!");
				}

				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}
				catch(JobNotDeletedException&)
				{
					SDPA_LOG_DEBUG("The JobManager could not delete the job "<<pEvt->job_id());
				}
			}
			else
				SDPA_LOG_ERROR("Gwes not initialized!");

		}
		catch(...) {
			SDPA_LOG_DEBUG("Unexpected exception occurred!");
		}
	}
}

void GenericDaemon::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	ostringstream os;
	SDPA_LOG_DEBUG("Call 'handleJobFailedEvent'");

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFailed(pEvt);
	}
	catch(JobNotFoundException){
		SDPA_LOG_DEBUG("Job "<<pEvt->job_id()<<" not found!");
	}

	if(pEvt->from() == sdpa::daemon::GWES ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES, this is a local job
		// if I'm not the orchestrator
		//send JobFinished event to the master if daemon == aggregator || NRE

		if( name() != ORCHESTRATOR ) // if I'm not an orchestrator forward the message up
		{
			try {
				// forward it up
				JobFailedEvent::Ptr pEvtJobFailedEvent(new JobFailedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

				// send the event to the master
				sendEventToMaster(pEvtJobFailedEvent, MSG_RETRY_CNT);
				// delete it from the map when you receive a JobFaileddAckEvent!
			}
			catch(QueueFull)
			{
				SDPA_LOG_DEBUG("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a SubmitJobEvent");
			}
			catch(seda::StageNotFound)
			{
				SDPA_LOG_DEBUG("Stage not found when trying to submit SubmitJobEvent");
			}
			catch(...) {
				SDPA_LOG_DEBUG("Unexpected exception occurred!");
			}
		}
		else
			SDPA_LOG_DEBUG("Don't notify the user! He will periodically query for the job status!");

		// no acknowledgement to be sent to GWES
	}
	else //event sent by a worker
	{
		Worker::worker_id_t worker_id = pEvt->from();

		if( name() != NRE )
		{
			// send a JobFailedAckEvent back to the worker/slave
			JobFailedAckEvent::Ptr pEvtJobFailedAckEvt(new JobFailedAckEvent(name(), worker_id, pEvt->job_id(), pEvt->id()));
			sendEventToSlave(pEvtJobFailedAckEvt);
		}

		try {
			// Should set the workflow_id here, or send it together with the workflow description
			if(ptr_Sdpa2Gwes_)
			{
				activity_id_t actId = pJob->id().str();
				workflow_id_t wfId  = pJob->parent().str();

				parameter_list_t output;

				SDPA_LOG_DEBUG("Inform GWES that the activity "<<actId<<" failed");

				try {
					ptr_Sdpa2Gwes_->activityFailed(wfId, actId, output);
				}
				catch(gwes::Gwes2Sdpa::NoSuchActivity& )
				{
					SDPA_LOG_ERROR("NoSuchActivityException occurred!");
				}

				try {
					// delete the job from the worker's queues
					Worker::ptr_t ptrWorker = findWorker(worker_id);
					// delete job from worker's queues

					SDPA_LOG_DEBUG("Delete the job "<<pEvt->job_id()<<" from the worker's queues!");
					ptrWorker->delete_job(pEvt->job_id());
				}
				catch(WorkerNotFoundException)
				{
					SDPA_LOG_DEBUG("Worker "<<worker_id<<" not found!");
				}

				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}
				catch(JobNotDeletedException&)
				{
					SDPA_LOG_DEBUG("The JobManager could not delete the job "<<pEvt->job_id());
				}
			}
			else
				SDPA_LOG_ERROR("Gwes not initialized!");

		}
		catch(...) {
			SDPA_LOG_DEBUG("Unexpected exception occurred!");
		}
	}
}

// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent(const JobFinishedAckEvent* pEvt)
{
	acknowledge(pEvt->id());
   // The result was succesfully delivered, so I can delete the job from the job map
	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a JobFinishedAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(JobNotFoundException)
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
	acknowledge(pEvt->id());
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job
	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a JobFinishedAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(JobNotFoundException)
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

void GenericDaemon::handleQueryJobStatusEvent(const QueryJobStatusEvent* pEvt )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->QueryJobStatus(pEvt); // should send back a message with the status
	}
	catch(JobNotFoundException)
	{
		ostringstream os;
		os<<"The job "<<pEvt->job_id()<<" was not found by the JobManager";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
	ostringstream os;
	os<<"Call 'handleCancelJobEvent'";
	SDPA_LOG_DEBUG(os.str());

	Job::ptr_t pJob;
	// put the job into the state Cancelling
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->CancelJob(pEvt);
		SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

		// The user or the GC does this!
		/*if(pJob->is_marked_for_deletion())
			ptr_job_man_->deleteJob(pEvt->job_id());*/

		//if master not set i.e. I'm the orchestrator send immediately an acknowledgment to the user
		// the event was generated by gwes and it corresponds to an activity
		// this activity might be already be assigned to  a worker but not yet sent or acknowledged
		if( name() == ORCHESTRATOR )
		{
			CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

			// only if the job was already submitted
			sendEventToMaster(pCancelAckEvt);
			os<<std::endl<<"Sent CancelJobAckEvent to the user "<<pEvt->from();
			SDPA_LOG_DEBUG(os.str());
		}
	}
	catch(JobNotFoundException){
		os.str("");
		os<<"Job "<<pEvt->job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	// transition from Cancelling to Cancelled
	// FIXME: is it really to safe to just acknowledge here?
	acknowledge(pEvt->id());

	ostringstream os;
	Worker::worker_id_t worker_id = pEvt->from();
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());

		// put the job into the state Cancelled
	    pJob->CancelJobAck(pEvt);
	    SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

    	// should send acknowlwdgement
    	if( pEvt->from() == sdpa::daemon::GWES  ) // the message comes from GWES, forward it to the master
		{
    		if( name()!= ORCHESTRATOR )
    		{
				os<<std::endl<<"Sent CancelJobAckEvent to "<<master();
				CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), master(), pEvt->job_id(), pEvt->id()));

				// only if the job was already submitted, send ack to master
				sendEventToMaster(pCancelAckEvt);

				// if I'm not the orchestrator delete effectively the job
				ptr_job_man_->deleteJob(pEvt->job_id());
    		}
		}
    	else // the message comes from an worker, forward it to GWES
    	{
    		try {
    			Worker::ptr_t ptrWorker = findWorker(worker_id);

				// in the message comes from a worker
				ptrWorker->delete_job(pEvt->job_id());
    		 }
    		 catch(WorkerNotFoundException)
    		 {
    			os.str("");
    			os<<"Worker "<<worker_id<<" not found!";
    			SDPA_LOG_DEBUG(os.str());
    		}

    		// tell to GWES that the activity job_id() was cancelled
    		activity_id_t actId = pJob->id();
    		workflow_id_t wfId  = pJob->parent();

    		// inform gwes that the activity was canceled
    		ptr_Sdpa2Gwes_->activityCanceled(wfId, actId);
    	}
	}
	catch(JobNotFoundException)
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

void GenericDaemon::handleRetrieveResultsEvent(const RetrieveJobResultsEvent* ptr )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
		pJob->RetrieveJobResults(ptr);
	}
	catch(JobNotFoundException)
	{
		ostringstream os;
		os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
		SDPA_LOG_DEBUG(os.str());
	}
}
