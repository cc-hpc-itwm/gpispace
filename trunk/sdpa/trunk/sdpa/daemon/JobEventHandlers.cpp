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
using namespace sdpa::events;

void GenericDaemon::handleSubmitJobAckEvent(const SubmitJobAckEvent* pEvent)
{
	ostringstream os;
	os<<"Call 'handleSubmitJobAckEvent'";
	SDPA_LOG_DEBUG(os.str());

	acknowledge(pEvent->id());

	Worker::worker_id_t worker_id = pEvent->from();
	try {
		ptr_scheduler_->acknowledgeJob(worker_id, pEvent->job_id());

	} catch(WorkerNotFoundException) {
		SDPA_LOG_DEBUG("Worker "<<worker_id<<" not found!");
		(os.str());
	} catch(...) {
		SDPA_LOG_DEBUG("Unexpected exception occurred!");
	}
}

void GenericDaemon::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
}

void GenericDaemon::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
}

void GenericDaemon::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
}

void GenericDaemon::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
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

void GenericDaemon::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* ptr )
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
