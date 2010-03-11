/*
 * =====================================================================================
 *
 *       Filename:  NRE.hpp
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

#ifndef SDPA_NRE_HPP
#define SDPA_NRE_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>

#include <sdpa/daemon/Observable.hpp>
#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>

typedef sdpa::daemon::NotificationService gui_service;


namespace sdpa {
	namespace daemon {
	  template <typename T>
	  class NRE : public dsm::DaemonFSM,  public sdpa::daemon::Observable {
	  public:
		typedef sdpa::shared_ptr<NRE<T> > ptr_t;
		SDPA_DECLARE_LOGGER();

		NRE( const std::string& name = "", const std::string& url = "",
			 const std::string& masterName = "", const std::string& masterUrl = "",
			 const std::string& workerUrl = "", const std::string& guiUrl = "",
			 const bool bExtSched = false )
		: dsm::DaemonFSM( name, dynamic_cast<IWorkflowEngine*>(new T(this)) ),
				  SDPA_INIT_LOGGER(name),
				  url_(url),
				  masterName_(masterName),
				  masterUrl_(masterUrl),
				  m_guiServ("SDPA", guiUrl)
		{
			SDPA_LOG_DEBUG("NRE constructor called ...");
			if(!bExtSched)
				ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerNRE(this, workerUrl));

			// attach gui observer
			SDPA_LOG_DEBUG("Attach GUI observer ...");
			attach_observer(&m_guiServ);
		}

		virtual ~NRE()
		{
			SDPA_LOG_DEBUG("NRE destructor called ...");
			daemon_stage_ = NULL;
			detach_observer( &m_guiServ );
		}

		static ptr_t create( const std::string& name, const std::string& url,
								  const std::string& masterName, const std::string& masterUrl,
								  const std::string& workerUrl,  const std::string guiUrl="127.0.0.1:9000",
								  const bool bExtSched = false )
		{
			 return ptr_t(new NRE<T>( name, url, masterName, masterUrl, workerUrl, guiUrl, bExtSched ));
		}

		static void start( NRE<T>::ptr_t ptrNRE );
		static void shutdown(NRE<T>::ptr_t ptrNRE );

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );
		void action_interrupt( const sdpa::events::InterruptEvent& );

		bool cancel(const id_type&, const reason_type & );

		void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

		const std::string& url() const {return url_;}
		const std::string& masterName() const { return masterName_; }
		const std::string& masterUrl() const { return masterUrl_; }

		/*void activityCreated(const gwes::activity_t& act);
		void activityStarted(const gwes::activity_t& act);
		void activityFinished(const gwes::activity_t& act);
		void activityFailed(const gwes::activity_t& act);
		void activityCancelled(const gwes::activity_t& act);*/

		template <class Archive>
		void serialize(Archive& ar, const unsigned int file_version )
		{
			ar & boost::serialization::base_object<DaemonFSM>(*this);
		}

		virtual void backup( const std::string& strArchiveName );
	    virtual void recover( const std::string& strArchiveName );

		friend class boost::serialization::access;
		friend class sdpa::tests::WorkerSerializationTest;


	  protected:
		const std::string url_;
		const std::string masterName_;
		const std::string masterUrl_;
		gui_service m_guiServ;
	  };
	}
}

// Implementation

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>


using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;


template <typename T>
void NRE<T>:: start(NRE<T>::ptr_t ptrNRE)
{
	dsm::DaemonFSM::create_daemon_stage(ptrNRE);
	ptrNRE->configure_network( ptrNRE->url(), ptrNRE->masterName(), ptrNRE->masterUrl() );
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrNRE, ptrCfg);
}

template <typename T>
void NRE<T>::shutdown(NRE<T>::ptr_t ptrNRE)
{
	ptrNRE->shutdown_network();
	ptrNRE->stop();

	delete ptrNRE->ptr_workflow_engine_;
	ptrNRE->ptr_workflow_engine_ = NULL;
}

template <typename T>
void NRE<T>::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_configure'");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put<sdpa::util::time_type>("polling interval",          50 * 1000); //0.1s
	ptr_daemon_cfg_->put<sdpa::util::time_type>("life-sign interval", 60 * 1000 * 1000); //60s
}

template <typename T>
void NRE<T>::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");

	SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
	to_master_stage()->send(pEvtWorkerReg);
}

template <typename T>
void NRE<T>::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

}

template <typename T>
void NRE<T>::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
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

template <typename T>
void NRE<T>::handleJobFailedEvent(const JobFailedEvent* pEvt )
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
template <typename T>
bool  NRE<T>::cancel(const id_type& activityId, const reason_type& reason )
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	/*job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);*/
}

/*
void NRE<T>::activityCreated(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_CREATED));
}

void NRE<T>::activityStarted(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_STARTED));
}

void NRE<T>::activityFinished(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_FINISHED));
}

void NRE<T>::activityFailed(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_FAILED));
}

void NRE<T>::activityCancelled(const gwes::activity_t& act)
{
	notifyObservers(NotificationEvent(act.getID(), act.getName(), NotificationEvent::STATE_CANCELLED));
}
*/

template <typename T>
void NRE<T>::backup( const std::string& strArchiveName )
{
	try
	{
		print();

		ptr_t ptrNRE_0(this);
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

template <typename T>
void NRE<T>::recover( const std::string& strArchiveName )
{
	try
	{
		ptr_t ptrRestoredNRE_0(this);
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

#endif //SDPA_NRE_HPP
