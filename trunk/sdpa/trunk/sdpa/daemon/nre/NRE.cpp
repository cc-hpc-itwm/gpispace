/*
 * =====================================================================================
 *
 *       Filename:  NRE.cpp
 *
 *    Description:  Contains the NRE class
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

// Implementation
//#include <sdpa/daemon/nre/NRE.hpp>
#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

template <typename U>
void NRE<U>:: start(NRE<U>::ptr_t ptrNRE)
{
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	ptrNRE->configure_network( ptrNRE->url(), ptrNRE->masterName() );
	ptrNRE->configure(ptrCfg);
}

template <typename U>
void NRE<U>::shutdown(NRE<U>::ptr_t ptrNRE)
{
	if(ptrNRE->bLaunchNrePcd_)
		ptrNRE->shutdownNrePcd();

	LOG(INFO, "shutting down...");
	ptrNRE->shutdown_network();
	ptrNRE->stop_stages();

	if ( ptrNRE->hasWorkflowEngine() )
	{
		DLOG(TRACE, "deleting workflow engine");
		delete ptrNRE->ptr_workflow_engine_;
		ptrNRE->ptr_workflow_engine_ = NULL;
	}
}

template <typename U>
void NRE<U>::action_configure(const StartUpEvent &se)
{
	GenericDaemon::action_configure (se);

	SDPA_LOG_INFO("Configuring myeself (nre)...");

	// should set/update this dynamically, as a function of number of workers and their
	// processing capacities

	ptr_daemon_cfg_->put("nmax_ext_job_req", 2U);
	ptr_daemon_cfg_->put("polling interval", 50 * 1000); //0.1s
}

template <typename U>
void NRE<U>::action_config_ok(const ConfigOkEvent& e)
{
        GenericDaemon::action_config_ok (e);

	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuration (nre) was ok");

	std::ostringstream sstr;
	ptr_daemon_cfg_->writeTo (sstr);
	SDPA_LOG_INFO("config: " << sstr.str());

	SDPA_LOG_INFO("NRE (" << name() << ") sending registration event to master (" << master() << ") my rank=" << rank());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master(), rank() ));
	sendEventToMaster (pEvtWorkerReg);
}

template <typename U>
void NRE<U>::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

}

template <typename U>
void NRE<U>::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
  assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

  DLOG(TRACE, "handleJobFinishedEvent(" << pEvt->job_id() << ")");

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished(pEvt);
	}
	catch(JobNotFoundException const &){
		SDPA_LOG_ERROR("Job "<<pEvt->job_id()<<" not found!");
		return;
	}

	if( pEvt->from() == sdpa::daemon::WE || !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
	{
		try {
			// forward it up
			JobFinishedEvent::Ptr pEvtJobFinished( new JobFinishedEvent(name(), master(), pEvt->job_id(), pEvt->result()) );
			// send the event to the master
			sendEventToMaster(pEvtJobFinished);
			// delete it from the map when you receive a JobFinishedAckEvent!
		}
		catch(QueueFull const &)
		{
			SDPA_LOG_ERROR("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFinishedEvent");
		}
		catch(seda::StageNotFound const &)
		{
            SDPA_LOG_ERROR("Stage not found when trying to submit JobFinishedEvent");
            throw;
		}
		catch(std::exception const & ex)
		{
			SDPA_LOG_ERROR("Exception during stage->send: " << ex.what());
            throw;
		}
		catch(...)
		{
            SDPA_LOG_ERROR("Unknown exception during stage->send!");
            throw;
		}
	}
        else
        {
          LOG(ERROR, "got JobFinished event from suspicious source: " << pEvt->from());
        }
}

template <typename U>
void NRE<U>::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
  assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

  DLOG(TRACE, "handleJobFailedEvent(" << pEvt->job_id() << ")");

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFailed(pEvt);
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR("Job "<<pEvt->job_id()<<" not found!");
        return;
	}

	if( pEvt->from() == sdpa::daemon::WE ||  !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
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
		catch(QueueFull const &)
		{
			SDPA_LOG_WARN("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFailedEvent");
		}
		catch(seda::StageNotFound const &)
		{
			SDPA_LOG_FATAL("Stage not found when trying to send JobFailedEvent");
			throw;
		}
		catch(std::exception const & ex)
		{
			SDPA_LOG_FATAL("Exception during stage->send: " << ex.what());
            throw;
		}
		catch(...) {
            SDPA_LOG_FATAL("Unknown exception during stage->send!");
            throw;
		}
	}
}

template <typename U>
void NRE<U>::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
	assert (pEvt);
	LOG(INFO, "cancelling job: " << pEvt->job_id());

	// put the job into the state Cancelling
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
        pJob->CancelJob(pEvt);
        DLOG(TRACE, "The job state is: "<<pJob->getStatus());
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR ("job " << pEvt->job_id() << " could not be found!");
        throw;
	}
}

template <typename U>
void NRE<U>::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt )
{
  assert(pEvt);

  DLOG(TRACE, "handleCancelJobAckEvent(" << pEvt->job_id() << ")");

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->CancelJobAck(pEvt);
	}
	catch(JobNotFoundException const &){
		SDPA_LOG_ERROR("Job "<<pEvt->job_id()<<" not found!");
                return;
	}

	if( pEvt->from() == sdpa::daemon::WE ||  !hasWorkflowEngine() ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES, this is a local job
		// if I'm not the orchestrator
		//send JobFinished event to the master if daemon == aggregator || NRE

		try {
			// forward it up
			const sdpa::events::SDPAEvent::message_id_type mid;
			CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(name(), master(), pEvt->job_id(), mid));

			// send the event to the master
			sendEventToMaster(pEvtCancelJobAck);
			// delete it from the map when you receive a JobCancelleddAckEvent!
		}
		catch(QueueFull const &)
		{
			SDPA_LOG_ERROR("Failed to send to the ,aster output stage "<<ptr_to_master_stage_->name()<<" CancelJobAckEvent");
		}
		catch(seda::StageNotFound const &)
		{
			SDPA_LOG_ERROR("Stage not found when trying to submit CancelJobAckEvent");
		}
		catch(...) {
			SDPA_LOG_ERROR("Unexpected exception occurred!");
		}
	}
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
template <typename U>
bool  NRE<U>::cancel(const id_type& activityId, const reason_type& /* reason */)
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	/*job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);*/

	return true;
}

template <typename U>
void NRE<U>::activityCreated( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_CREATED) );
}

template <typename U>
void NRE<U>::activityStarted( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_STARTED));
}

template <typename U>
void NRE<U>::activityFinished( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers(NotificationEvent(id, act_name, NotificationEvent::STATE_FINISHED));
}

template <typename U>
void NRE<U>::activityFailed( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_FAILED) );
}

template <typename U>
void NRE<U>::activityCancelled( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_CANCELLED) );
}

template <typename U>
void NRE<U>::backup( const std::string& strArchiveName )
{
	try
	{
          print();
          ptr_t ptrNRE_0(this);
          std::ofstream ofs(strArchiveName.c_str());
          boost::archive::text_oarchive oa(ofs);
          oa.register_type(static_cast<NRE<U>*>(NULL));
          //oa.register_type(static_cast<T*>(NULL));
          oa.register_type(static_cast<DaemonFSM*>(NULL));
          oa.register_type(static_cast<GenericDaemon*>(NULL));
          oa.register_type(static_cast<SchedulerImpl*>(NULL));
          //oa.register_type(static_cast<SchedulerNRE<sdpa::nre::worker::NreWorkerClient>*>(NULL));
          oa.register_type(static_cast<SchedulerNRE<U>*>(NULL));
          oa.register_type(static_cast<JobFSM*>(NULL));
          //oa.register_type(static_cast<sdpa::daemon::NRE*>(NULL));
          oa << ptrNRE_0;
	}
	catch(exception &e)
	{
          cout <<"Exception occurred: "<< e.what() << endl;
	}
}

template <typename U>
void NRE<U>::recover( const std::string& strArchiveName )
{
	try
	{
		ptr_t ptrRestoredNRE_0(this);
		std::ifstream ifs(strArchiveName.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<NRE<U>*>(NULL));
		//ia.register_type(static_cast<T*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		//ia.register_type(static_cast<SchedulerNRE<sdpa::nre::worker::NreWorkerClient>*>(NULL));
		ia.register_type(static_cast<SchedulerNRE<U>*>(NULL));
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

template <typename U>
void NRE<U>::shutdownNrePcd()
{

	int c;
	int nStatus;

	//kill pcd here
	SchedulerNRE<U>* pSchedNRE = dynamic_cast<SchedulerNRE<U>*>(ptr_scheduler_.get());
	if( pSchedNRE )
	{
		pid_t pidPcd = pSchedNRE->nre_worker_client().getPcdPid();
		//pid_t pidPcd = dynamic_pointer_cast<sdpa::shared_ptr<SchedulerNRE, Scheduler::ptr_t>(ptr_scheduler_)->nre_worker_client().getPcdPid();

		if (pidPcd <= 1)
		{
			LOG(ERROR, "cannot send TERM signal to child with pid: " << pidPcd);
			throw std::runtime_error ("pcd does not have a valid pid (<= 1)");
		}

		kill(pidPcd, SIGTERM);

		c = wait(&nStatus);
		if( WIFEXITED(nStatus) )
		{
			if( WEXITSTATUS(nStatus) != 0 )
			{
				std::cerr<<"nre-pcd exited with the return code "<<(int)WEXITSTATUS(nStatus)<<endl;
				LOG(ERROR, "nre-pcd exited with the return code "<<(int)WEXITSTATUS(nStatus));
			}
			else
				if( WIFSIGNALED(nStatus) )
				{
					std::cerr<<"nre-pcd exited due to a signal: " <<(int)WTERMSIG(nStatus)<<endl;
					LOG(ERROR, "nre-pcd exited due to a signal: "<<(int)WTERMSIG(nStatus));
				}
		}
				}
}

