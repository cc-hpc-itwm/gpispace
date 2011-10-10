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
#include <sdpa/daemon/mpl.hpp>

#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>

#include <sdpa/daemon/exceptions.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

void GenericDaemon::handleSubmitJobAckEvent(const SubmitJobAckEvent* pEvent)
{
	DLOG(TRACE, "handleSubmitJobAckEvent: " << pEvent->job_id() << " from " << pEvent->from());

	Worker::worker_id_t worker_id = pEvent->from();
	try {
		// Only, now should be state of the job updated to RUNNING
		// since it was not rejected, no error occurred etc ....
		//find the job ptrJob and call
		Job::ptr_t ptrJob = ptr_job_man_->findJob(pEvent->job_id());
		ptrJob->Dispatch();
		ptr_scheduler_->acknowledgeJob(worker_id, pEvent->job_id());
	} catch(WorkerNotFoundException const &)
	{
		SDPA_LOG_ERROR("job submission could not be acknowledged: worker " << worker_id << " not found!!");
	}
	catch(std::exception const &ex)
	{
		SDPA_LOG_ERROR("Unexpected exception occurred during submitJobAck: " << ex.what());
	}
}

void GenericDaemon::handleJobFinishedEvent(const JobFinishedEvent* /* pEvt */)
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
}

void GenericDaemon::handleJobFailedEvent(const JobFailedEvent* /* pEvt */ )
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
}

void GenericDaemon::handleCancelJobEvent(const CancelJobEvent* /* pEvt */ )
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");
}

void GenericDaemon::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt )
{
	SDPA_LOG_DEBUG("Not implemented! Should be overridden by the daemons.");

	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a CancelJobAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");
	}
	catch(JobNotDeletedException const & ex)
	{
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex.what());
	}
	catch(...) {
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not deleted, unexpected error!");
	}
}

// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent(const JobFinishedAckEvent* pEvt)
{
    // The result was successfully delivered by the worker and the WE was notified
	//therefore, I can delete the job from the job map
	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a JobFinishedAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");
	}
	catch(JobNotDeletedException const & ex)
	{
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex.what());
	}
	catch(...) {
        SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not deleted, unexpected error!");
	}
}


// respond to a worker that the JobFailedEvent was received
void GenericDaemon::handleJobFailedAckEvent(const JobFailedAckEvent* pEvt )
{
	ostringstream os;
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		// delete it from the map when you receive a JobFinishedAckEvent!
		ptr_job_man_->deleteJob(pEvt->job_id());
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");
	}
	catch(JobNotDeletedException const & ex)
	{
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex.what());
	}
	catch(...) {
		SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not deleted, unexpected error!");
	}
}

void GenericDaemon::handleQueryJobStatusEvent(const QueryJobStatusEvent* pEvt )
{
	sdpa::job_id_t jobId = pEvt->job_id();

	try {

		Job::ptr_t pJob = ptr_job_man_->findJob(jobId);
		//SDPA_LOG_INFO("The job "<<jobId<<" has the status "<<pJob->getStatus());
		pJob->QueryJobStatus(pEvt, this); // should send back a message with the status
	}
	catch(JobNotFoundException const& ex)
	{
		SDPA_LOG_WARN("Couldn't find the job " <<jobId<< "!");
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, ex.what()) );
		sendEventToMaster(pErrorEvt);
	}
}

void GenericDaemon::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* pEvt )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->RetrieveJobResults(pEvt, this);
	}
	catch(JobNotFoundException const& ex)
	{
		SDPA_LOG_WARN("Couldn't find the job " << pEvt->job_id() << "!");
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, ex.what()) );
		sendEventToMaster(pErrorEvt);
	}
}
