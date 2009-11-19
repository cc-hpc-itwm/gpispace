/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.cpp
 *
 *    Description:  Specific Implementation for the orchestrator
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

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>
#include <SchedulerOrch.hpp>
#include <Orchestrator.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

Orchestrator::Orchestrator(  const std::string &name,  const std::string& url )
	: DaemonFSM( name, new gwes::GWES() ),
	  SDPA_INIT_LOGGER(name),
	  url_(url)
{
	SDPA_LOG_DEBUG("Orchestrator constructor called ...");
	ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerOrch(this));
}

Orchestrator::~Orchestrator()
{
	SDPA_LOG_DEBUG("Orchestrator destructor called ...");
	daemon_stage_ = NULL;
}

Orchestrator::ptr_t Orchestrator::create( const std::string& name, const std::string& url )
{
	return Orchestrator::ptr_t(new Orchestrator(name, url));
}

void Orchestrator::start( Orchestrator::ptr_t ptrOrch )
{
	dsm::DaemonFSM::create_daemon_stage(ptrOrch);
	ptrOrch->configure_network( ptrOrch->url() );
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrOrch, ptrCfg);
}

void Orchestrator::shutdown( Orchestrator::ptr_t ptrOrch )
{
	ptrOrch->shutdown_network();
	ptrOrch->stop();

	delete ptrOrch->ptr_Sdpa2Gwes_;
	ptrOrch->ptr_Sdpa2Gwes_ = NULL;
}

//actions
void Orchestrator::action_configure(const StartUpEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_configure'");
}

void Orchestrator::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");

}

void Orchestrator::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
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

	if(pEvt->from() != sdpa::daemon::GWES ) // use a predefined variable here of type enum or use typeid
	{
		Worker::worker_id_t worker_id = pEvt->from();
		try {
			// Should set the workflow_id here, or send it together with the workflow description
			if(ptr_Sdpa2Gwes_)
			{
				activity_id_t actId = pJob->id().str();
				workflow_id_t wfId  = pJob->parent().str();

				gwes::Activity& gwes_act = (gwes::Activity&)ptr_Sdpa2Gwes_->getActivity(wfId, actId);

				try
				{
					sdpa::wf::glue::put_results_to_activity(pEvt->result(), gwes_act);
				}
				catch (const std::exception &ex)
				{
					SDPA_LOG_ERROR("could not put results back to activity: "<< ex.what());
				}

				parameter_list_t output = *gwes_act.getTransitionOccurrence()->getTokens();

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

					// send a JobFinishedAckEvent back to the worker/slave
					//delete it also from job_map_
					JobFinishedAckEvent::Ptr pEvtJobFinishedAckEvt(new JobFinishedAckEvent(name(), worker_id, pEvt->job_id()));

					// send the event to the slave
					sendEvent(ptr_to_slave_stage_, pEvtJobFinishedAckEvt);
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

void Orchestrator::handleJobFailedEvent(const JobFailedEvent* pEvt )
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

	if( pEvt->from() != sdpa::daemon::GWES ) // use a predefined variable here of type enum or use typeid
	{
		Worker::worker_id_t worker_id = pEvt->from();
		try {
			// Should set the workflow_id here, or send it together with the workflow description
			if(ptr_Sdpa2Gwes_)
			{
				activity_id_t actId = pJob->id().str();
				workflow_id_t wfId  = pJob->parent().str();

				gwes::Activity& gwes_act = (gwes::Activity&)ptr_Sdpa2Gwes_->getActivity(wfId, actId);

				try
				{
					sdpa::wf::glue::put_results_to_activity(pEvt->result(), gwes_act);
				}
				catch (const std::exception &ex)
				{
					SDPA_LOG_ERROR("could not put results back to activity: "<< ex.what());
				}

				parameter_list_t output = *gwes_act.getTransitionOccurrence()->getTokens();

				SDPA_LOG_DEBUG("Inform GWES that the activity "<<actId<<" failed");

				try {
					ptr_Sdpa2Gwes_->activityFailed(wfId, actId, output);
				}
				catch(gwes::Gwes2Sdpa::NoSuchActivity& )
				{
					SDPA_LOG_ERROR("NoSuchActivityException occurred!");
				}

				try {
					Worker::ptr_t ptrWorker = findWorker(worker_id);
					// delete job from worker's queues

					SDPA_LOG_DEBUG("Delete the job "<<pEvt->job_id()<<" from the worker's queues!");
					ptrWorker->delete_job(pEvt->job_id());

					// send a JobFailedAckEvent back to the worker/slave
					//delete it also from job_map_
					JobFailedAckEvent::Ptr pEvtJobFailedAckEvt(new JobFailedAckEvent(name(), worker_id, pEvt->job_id()));

					// send the event to the slave
					sendEvent(ptr_to_slave_stage_, pEvtJobFailedAckEvt);
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

void Orchestrator::handleCancelJobEvent(const CancelJobEvent* pEvt )
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

		CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id()));

		// only if the job was already submitted
		sendEvent(to_master_stage(), pCancelAckEvt);
		os<<std::endl<<"Sent CancelJobAckEvent to the user "<<pEvt->from();
		SDPA_LOG_DEBUG(os.str());
	}
	catch(JobNotFoundException){
		os.str("");
		os<<"Job "<<pEvt->job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void Orchestrator::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	// transition from Cancelling to Cancelled

	ostringstream os;
	Worker::worker_id_t worker_id = pEvt->from();
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());

		// put the job into the state Cancelled
	    pJob->CancelJobAck(pEvt);
	    SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

    	// should send acknowlwdgement
    	if( pEvt->from() != sdpa::daemon::GWES  ) // the message comes from GWES, forward it to the master
    	{
    		try {
    			Worker::ptr_t ptrWorker = findWorker(worker_id);

				// in the message comes from a worker
				ptrWorker->delete_job(pEvt->job_id());
    		}
    		catch(WorkerNotFoundException) {
    			SDPA_LOG_DEBUG("Worker "<<worker_id<<" not found!");
    		}

    		// tell to GWES that the activity ob_id() was cancelled
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
