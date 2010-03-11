/*
 * =====================================================================================
 *
 *       Filename:  NRE.cpp
 *
 *    Description:  Implements the NRE
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

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <sdpa/daemon/nre/NRE.hpp>
#include <tests/sdpa/DummyGwes.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

NRE :: NRE(  const std::string& name, const std::string& url,
			 const std::string& masterName, const std::string& masterUrl,
			 const std::string& workerUrl,
			 const std::string& guiUrl,
			 const bool bExtSched, const bool bUseDummyWE )
		: dsm::DaemonFSM( name, (bUseDummyWE ? dynamic_cast<IWorkflowEngine*>(new DummyGwes(this)) : NULL) ),
		  SDPA_INIT_LOGGER(name),
		  url_(url),
		  masterName_(masterName),
		  masterUrl_(masterUrl),
		  m_guiServ("SDPA", guiUrl)
{
	SDPA_LOG_DEBUG("NRE constructor called ...");
	if(!bExtSched)
		ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerNRE(this, workerUrl));

	//attach gui observer
	SDPA_LOG_DEBUG("Attach GUI observer ...");
	attach_observer(&m_guiServ);
}

NRE :: ~NRE()
{
	SDPA_LOG_DEBUG("NRE destructor called ...");
	daemon_stage_ = NULL;
	detach_observer( &m_guiServ );
}

NRE::ptr_t NRE ::create( const std::string& name, const std::string& url,
						 const std::string& masterName, const std::string& masterUrl,
						 const std::string& workerUrl,
						 const std::string guiUrl,
						 const bool bExtSched, const bool bUseDummyWE)
{
	 return NRE::ptr_t(new NRE( name, url, masterName, masterUrl, workerUrl, guiUrl, bExtSched, bUseDummyWE ));
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

	delete ptrNRE->ptr_workflow_engine_;
	ptrNRE->ptr_workflow_engine_ = NULL;
}

//actions
void NRE::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_configure'");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put<sdpa::util::time_type>("polling interval",          50 * 1000); //0.1s
	ptr_daemon_cfg_->put<sdpa::util::time_type>("life-sign interval", 60 * 1000 * 1000); //60s
}

void NRE::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");

	SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
	to_master_stage()->send(pEvtWorkerReg);
}

void NRE::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

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
			sendEventToMaster(pEvtJobFailedEvent);
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


/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
bool  NRE::cancel(const id_type& activityId, const reason_type& reason )
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	/*job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);*/
}

/*
void NRE ::activityCreated(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_CREATED));
}

void NRE ::activityStarted(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_STARTED));
}

void NRE ::activityFinished(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_FINISHED));
}

void NRE ::activityFailed(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_FAILED));
}

void NRE ::activityCancelled(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_CANCELLED));
}
*/

void NRE::backup( const std::string& strArchiveName )
{
	try
	{
		print();

		NRE::ptr_t ptrNRE_0(this);
		std::ofstream ofs(strArchiveName.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<SchedulerNRE*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		//oa.register_type(static_cast<sdpa::daemon::NRE*>(NULL));
		oa << ptrNRE_0;
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: "<< e.what() << endl;
	}
}

void NRE::recover( const std::string& strArchiveName )
{
	try
	{
		NRE::ptr_t ptrRestoredNRE_0(this);
		std::ifstream ifs(strArchiveName.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<SchedulerNRE*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		//ia.register_type(static_cast<sdpa::daemon::NRE*>(NULL));
		ia >> ptrRestoredNRE_0;

		std::cout<<std::endl<<"----------------The restored content of the NRE is:----------------"<<std::endl;
		print();
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: " << e.what() << endl;
	}
}
