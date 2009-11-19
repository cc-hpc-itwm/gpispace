/*
 * =====================================================================================
 *
 *       Filename:  NRE.cpp
 *
 *    Description:  Implements the NRE
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
#include "SchedulerNRE.hpp"
#include <gwes/GWES.h>
#include <NRE.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

NRE :: NRE(  const std::string& name, const std::string& url,
			 const std::string& masterName, const std::string& masterUrl,
			 const std::string& workerUrl  )
		: dsm::DaemonFSM( name, new gwes::GWES() ),
		  SDPA_INIT_LOGGER(name),
		  url_(url),
		  masterName_(masterName),
		  masterUrl_(masterUrl)
{
	SDPA_LOG_DEBUG("NRE constructor called ...");
	ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerNRE(this, workerUrl));
}

NRE :: ~NRE()
{
	SDPA_LOG_DEBUG("NRE destructor called ...");
	daemon_stage_ = NULL;
}

NRE::ptr_t NRE ::create( const std::string& name, const std::string& url,
						 const std::string& masterName, const std::string& masterUrl,
						 const std::string& workerUrl )
{
	 return NRE::ptr_t(new NRE( name, url, masterName, masterUrl, workerUrl ));
}

void NRE :: start(NRE::ptr_t ptrNRE)
{
	dsm::DaemonFSM::create_daemon_stage(ptrNRE);
	ptrNRE->configure_network( ptrNRE->url(), ptrNRE->masterName(), ptrNRE->masterUrl() );
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrNRE, ptrCfg);
}

void NRE ::shutdown(NRE::ptr_t ptrNRE)
{
	ptrNRE->shutdown_network();
	ptrNRE->stop();

	delete ptrNRE->ptr_Sdpa2Gwes_;
	ptrNRE->ptr_Sdpa2Gwes_ = NULL;
}

//actions
void NRE::action_configure(const StartUpEvent& evt)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_configure'");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put<sdpa::util::time_type>("polling interval", 1000000); //1ms
	ptr_daemon_cfg_->put<sdpa::util::time_type>("life-sign interval", 1000000); //1s
}

void NRE::action_config_ok(const ConfigOkEvent& evt)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");

	SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
	to_master_stage()->send(pEvtWorkerReg);
}

void NRE::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

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
			sendEvent(ptr_to_master_stage_, pEvtJobFinished);
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
}

void NRE::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

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
		// the message comes from GWES, this is a local job
		// if I'm not the orchestrator
		//send JobFinished event to the master if daemon == aggregator || NRE

		try {
			// forward it up
			JobFailedEvent::Ptr pEvtJobFailedEvent(new JobFailedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEvent(ptr_to_master_stage_, pEvtJobFailedEvent);
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
}

gwes::activity_id_t  NRE ::submitActivity(gwes::activity_t &activity)
{
	SDPA_LOG_DEBUG("NRE GWES submitted new activity ...");
	ostringstream os;
	gwes::activity_id_t actId = activity.getID();
	gwes::workflow_id_t wfId  = activity.getOwnerWorkflowID();

	try {

		SDPA_LOG_DEBUG("Notify NRE GWES that the activity was dispatched ...");
		ptr_Sdpa2Gwes_->activityDispatched( wfId, actId );
		ptr_scheduler_->schedule(activity);
	}
	catch(std::exception&)
	{
		SDPA_LOG_DEBUG("Cancel the activity!");
		// inform immediately GWES that the corresponding activity was cancelled
		gwes()->activityCanceled( wfId, actId );
	}

	return activity.getID();
}

void NRE ::cancelActivity(const gwes::activity_id_t &activityId) throw (gwes::Gwes2Sdpa::NoSuchActivity)
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	job_id_t job_id(activityId);

	//find a way to kill an activity!!!
}

