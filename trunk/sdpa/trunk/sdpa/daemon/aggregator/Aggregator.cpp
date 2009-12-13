/*
 * =====================================================================================
 *
 *       Filename:  Aggreagtor.cpp
 *
 *    Description:  Specific Implementation for the aggregator
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
#include <SchedulerAgg.hpp>
#include <Aggregator.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;


Aggregator::Aggregator( const std::string& name, const std::string& url,
						const std::string& masterName, const std::string& masterUrl)
	: DaemonFSM( name, new gwes::GWES() ),
	  SDPA_INIT_LOGGER(name),
	  url_(url),
	  masterName_(masterName),
	  masterUrl_(masterUrl)
{
	SDPA_LOG_DEBUG("Aggregator constructor called ...");
	ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerAgg(this));
}

Aggregator::~Aggregator()
{
	SDPA_LOG_DEBUG("Aggregator destructor called ...");
	daemon_stage_ = NULL;
}

Aggregator::ptr_t Aggregator::create( const std::string& name, const std::string& url,
									  const std::string& masterName, const std::string& masterUrl )
{
	 return Aggregator::ptr_t( new Aggregator( name, url, masterName, masterUrl ));
}

void Aggregator::start(Aggregator::ptr_t ptrAgg)
{
	dsm::DaemonFSM::create_daemon_stage(ptrAgg);
	ptrAgg->configure_network( ptrAgg->url(), ptrAgg->masterName(), ptrAgg->masterUrl());
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrAgg, ptrCfg);
}

void Aggregator::shutdown(Aggregator::ptr_t ptrAgg)
{
	ptrAgg->shutdown_network();
	ptrAgg->stop();

	delete ptrAgg->ptr_Sdpa2Gwes_;
	ptrAgg->ptr_Sdpa2Gwes_ = NULL;
}

//actions
void Aggregator::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_configure'");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put<sdpa::util::time_type>("polling interval", 1000000); //1s
	ptr_daemon_cfg_->put<sdpa::util::time_type>("life-sign interval", 1000000); //1s
}

void Aggregator::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");
	// in fact the master name should be red from the configuration file

	SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
	to_master_stage()->send(pEvtWorkerReg);
}

void Aggregator::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
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

	if( pEvt->from() == sdpa::daemon::GWES ) // use a predefined variable here of type enum or use typeid
	{
		try {
			// forward it up
			JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEventToMaster(pEvtJobFinished);
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
	else //event sent by a worker
	{
		Worker::worker_id_t worker_id = pEvt->from();

		// send a JobFinishedAckEvent back to the worker/slave
		JobFinishedAckEvent::Ptr pEvtJobFinishedAckEvt(new JobFinishedAckEvent(name(), worker_id, pEvt->job_id(), pEvt->id()));

		// send the event to the slave
		sendEventToSlave(pEvtJobFinishedAckEvt);

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

void Aggregator::handleJobFailedEvent(const JobFailedEvent* pEvt )
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

	if( pEvt->from() == sdpa::daemon::GWES ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES
		try {
			// forward it up
			JobFailedEvent::Ptr pEvtJobFailedEvent(new JobFailedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEventToMaster(pEvtJobFailedEvent);
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
	else //event sent by a worker
	{
		Worker::worker_id_t worker_id = pEvt->from();

		// send a JobFailedAckEvent back to the worker/slave
		JobFailedAckEvent::Ptr pEvtJobFailedAckEvt(new JobFailedAckEvent(name(), worker_id, pEvt->job_id(), pEvt->id()));

		// send the event to the slave
		sendEventToSlave(pEvtJobFailedAckEvt);

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

				} catch(WorkerNotFoundException) {
					SDPA_LOG_DEBUG("Worker "<<worker_id<<" not found!");
				}

				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}catch(JobNotDeletedException&){
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

void Aggregator::handleCancelJobEvent(const CancelJobEvent* pEvt )
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
	}
	catch(JobNotFoundException){
		os.str("");
		os<<"Job "<<pEvt->job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void Aggregator::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
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
    	if( pEvt->from() == sdpa::daemon::GWES  ) // the message comes from GWES, forward it to the master
		{
			os<<std::endl<<"Sent CancelJobAckEvent to "<<master();
			CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), master(), pEvt->job_id(), pEvt->id()));

			// only if the job was already submitted, send ack to master
			sendEventToMaster(pCancelAckEvt);

			// if I'm not the orchestrator delete effectively the job
			ptr_job_man_->deleteJob(pEvt->job_id());

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

