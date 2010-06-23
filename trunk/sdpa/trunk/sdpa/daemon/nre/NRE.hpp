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
#include <sdpa/daemon/nre/NreWorkerClient.hpp>

#include <boost/pointer_cast.hpp>

#include <sys/wait.h>

using namespace std;


typedef sdpa::daemon::NotificationService gui_service;


namespace sdpa {
	namespace daemon {
	  template <typename T, typename U>
	  class NRE : public dsm::DaemonFSM,  public sdpa::daemon::Observable {
	  public:
		typedef sdpa::shared_ptr<NRE<T, U> > ptr_t;
		typedef typename T::internal_id_type we_internal_id_t;
		SDPA_DECLARE_LOGGER();

		NRE( const std::string& name = "",
			 const std::string& url = "",
			 const std::string& masterName = "",
			 const std::string& masterUrl = "",
			 const std::string& workerUrl = "",
			 const std::string& guiUrl = "",
			 bool bLaunchNrePcd = false,
			 const char* szNrePcdBinPath = "",
			 const char* szKDMModulesPath = "",
			 const char* szFvmPCModule = "")
		: dsm::DaemonFSM( name,  create_workflow_engine<T>() ),
				  SDPA_INIT_LOGGER(name),
				  url_(url),
				  masterName_(masterName),
				  masterUrl_(masterUrl),
				  //workerUrl_(workerUrl),
				  m_guiServ("SDPA", guiUrl)
		{
			SDPA_LOG_DEBUG("NRE constructor called ...");

			ptr_scheduler_ = sdpa::daemon::Scheduler::ptr_t(new SchedulerNRE<U>( this,
					                                                             workerUrl,
					                                                             bLaunchNrePcd,
					                                                             szNrePcdBinPath,
					                                                             szKDMModulesPath,
					                                                             szFvmPCModule ));

			// attach gui observer
			SDPA_LOG_DEBUG("Attach GUI observer ...");
			attach_observer(&m_guiServ);
		}

		virtual ~NRE()
		{
			SDPA_LOG_DEBUG("NRE destructor called ...");
			daemon_stage_ = NULL;
			detach_observer( &m_guiServ );

			/*int c;
		   	int nStatus;

			//kill pcd here
		   	kill(0,SIGTERM);

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
			*/

		}

		static ptr_t create( const std::string& name, const std::string& url,
								  const std::string& masterName, const std::string& masterUrl,
								  const std::string& workerUrl,  const std::string guiUrl="127.0.0.1:9000",
								  bool bLaunchNrePcd = false,  const char* szNrePcdBinPath = "",
								  const char* szKDMModulesPath = "", const char* szFvmPCModule = "")
		{
			 return ptr_t(new NRE<T, U>( name, url, masterName, masterUrl, workerUrl, guiUrl, bLaunchNrePcd,
					                     szNrePcdBinPath, szKDMModulesPath, szFvmPCModule ));
		}

      	static void start( NRE<T, U>::ptr_t ptrNRE );
		static void shutdown(NRE<T, U>::ptr_t ptrNRE );

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );
		void action_interrupt( const sdpa::events::InterruptEvent& );

		bool cancel(const id_type&, const reason_type & );

		void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );
		void handleCancelJobAckEvent(const CancelJobAckEvent* pEvt );

		void activityCreated( const id_type& id, const std::string& data );
		void activityStarted( const id_type& id, const std::string& data );
		void activityFinished( const id_type& id, const std::string& data );
		void activityFailed( const id_type& id, const std::string& data );
		void activityCancelled( const id_type& id, const std::string& data );

		const std::string& url() const {return url_;}
		const std::string& masterName() const { return masterName_; }
		const std::string& masterUrl() const { return masterUrl_; }

		template <class Archive>
		void serialize(Archive& ar, const unsigned int file_version )
		{
			ar & boost::serialization::base_object<DaemonFSM>(*this);
			ar & url_; //boost::serialization::make_nvp("url_", url_);
			ar & masterName_; //boost::serialization::make_nvp("url_", masterName_);
			ar & masterUrl_; //boost::serialization::make_nvp("url_", masterUrl_);
			//ar & workerUrl_;
			//ar & m_guiServ;
		}

		virtual void backup( const std::string& strArchiveName );
	    virtual void recover( const std::string& strArchiveName );

		friend class boost::serialization::access;
		friend class sdpa::tests::WorkerSerializationTest;


	  protected:

		Scheduler* create_scheduler()
		{
			return NULL; //new SchedulerNRE<U>(this);
		}

		std::string url_;
		std::string masterName_;
		std::string masterUrl_;
		//std::string workerUrl_;
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


template <typename T, typename U>
void NRE<T, U>:: start(NRE<T, U>::ptr_t ptrNRE)
{
	dsm::DaemonFSM::create_daemon_stage(ptrNRE);
	ptrNRE->configure_network( ptrNRE->url(), ptrNRE->masterName(), ptrNRE->masterUrl() );
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrNRE, ptrCfg);
}

template <typename T, typename U>
void NRE<T, U>::shutdown(NRE<T, U>::ptr_t ptrNRE)
{
	ptrNRE->shutdown_network();
	ptrNRE->stop();

	delete ptrNRE->ptr_workflow_engine_;
	ptrNRE->ptr_workflow_engine_ = NULL;
}

template <typename T, typename U>
void NRE<T, U>::action_configure(const StartUpEvent &se)
{
	GenericDaemon::action_configure (se);

	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuring myeself (nre)...");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put("polling interval",          50 * 1000); //0.1s
}

template <typename T, typename U>
void NRE<T, U>::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuration (nre) was ok");
	{
	  std::ostringstream sstr;
	  ptr_daemon_cfg_->writeTo (sstr);
	  SDPA_LOG_INFO("config: " << sstr.str());
	}

	SDPA_LOG_INFO("NRE (" << name() << ") sending registration event to master (" << master() << ") my rank=" << rank());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master(), rank() ));
	to_master_stage()->send(pEvtWorkerReg);
}

template <typename T, typename U>
void NRE<T, U>::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

}

template <typename T, typename U>
void NRE<T, U>::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
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

	if( pEvt->from() == sdpa::daemon::WE ) // use a predefined variable here of type enum or use typeid
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
			SDPA_LOG_DEBUG("Failed to send to the ,aster output stage "<<ptr_to_master_stage_->name()<<" a JobFinishedEvent");
		}
		catch(seda::StageNotFound)
		{
			SDPA_LOG_DEBUG("Stage not found when trying to submit JobFinishedEvent");
		}
		catch(...) {
			SDPA_LOG_DEBUG("Unexpected exception occurred!");
		}
	}
}

template <typename T, typename U>
void NRE<T, U>::handleJobFailedEvent(const JobFailedEvent* pEvt )
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

	if( pEvt->from() == sdpa::daemon::WE ) // use a predefined variable here of type enum or use typeid
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
			SDPA_LOG_DEBUG("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFailedEvent");
		}
		catch(seda::StageNotFound)
		{
			SDPA_LOG_DEBUG("Stage not found when trying to send JobFailedEvent");
		}
		catch(...) {
			SDPA_LOG_DEBUG("Unexpected exception occurred!");
		}
	}
}

template <typename T, typename U>
void NRE<T, U>::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt )
{
	SDPA_LOG_DEBUG("Call 'handleJobFailedEvent'");

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->CancelJobAck(pEvt);
	}
	catch(JobNotFoundException){
		SDPA_LOG_DEBUG("Job "<<pEvt->job_id()<<" not found!");
	}

	if( pEvt->from() == sdpa::daemon::WE ) // use a predefined variable here of type enum or use typeid
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
			// delete it from the map when you receive a JobFaileddAckEvent!
		}
		catch(QueueFull)
		{
			SDPA_LOG_DEBUG("Failed to send to the ,aster output stage "<<ptr_to_master_stage_->name()<<" CancelJobAckEvent");
		}
		catch(seda::StageNotFound)
		{
			SDPA_LOG_DEBUG("Stage not found when trying to submit CancelJobAckEvent");
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
template <typename T, typename U>
bool  NRE<T, U>::cancel(const id_type& activityId, const reason_type& reason )
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	/*job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);*/

	return true;
}

/*
template <typename T, typename U>
void NRE<T, U>::activityCreated( const id_type& id, const std::string& data )
{
	notifyObservers( NotificationEvent( id, data, NotificationEvent::STATE_CREATED) );
}

template <typename T, typename U>
void NRE<T, U>::activityStarted( const id_type& id, const std::string& data )
{
	notifyObservers( NotificationEvent( id, data, NotificationEvent::STATE_STARTED) );
}

template <typename T, typename U>
void NRE<T, U>::activityFinished( const id_type& id, const std::string& data )
{
	notifyObservers( NotificationEvent( id, data, NotificationEvent::STATE_FINISHED) );
}

template <typename T, typename U>
void NRE<T, U>::activityFailed( const id_type& id, const std::string& data )
{
	notifyObservers( NotificationEvent( id, data, NotificationEvent::STATE_FAILED) );
}

template <typename T, typename U>
void NRE<T, U>::activityCancelled( const id_type& id, const std::string& data )
{
	notifyObservers( NotificationEvent( id, data, NotificationEvent::STATE_CANCELLED) );
}*/

template <typename T, typename U>
void NRE<T, U>::activityCreated( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_CREATED) );
}

template <typename T, typename U>
void NRE<T, U>::activityStarted( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_STARTED));
}

template <typename T, typename U>
void NRE<T, U>::activityFinished( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers(NotificationEvent(id, act_name, NotificationEvent::STATE_FINISHED));
}

template <typename T, typename U>
void NRE<T, U>::activityFailed( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_FAILED) );
}

template <typename T, typename U>
void NRE<T, U>::activityCancelled( const id_type& id, const std::string& data )
{
 	const std::string act_name (we::util::text_codec::decode<we::activity_t> (data).transition().name());
 	notifyObservers( NotificationEvent( id, act_name, NotificationEvent::STATE_CANCELLED) );
}

template <typename T, typename U>
void NRE<T, U>::backup( const std::string& strArchiveName )
{
	try
	{
		print();

		ptr_t ptrNRE_0(this);
		std::ofstream ofs(strArchiveName.c_str());
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<NRE<T, U>*>(NULL));
		oa.register_type(static_cast<T*>(NULL));
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

template <typename T, typename U>
void NRE<T, U>::recover( const std::string& strArchiveName )
{
	try
	{
		ptr_t ptrRestoredNRE_0(this);
		std::ifstream ifs(strArchiveName.c_str());
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<NRE<T, U>*>(NULL));
		ia.register_type(static_cast<T*>(NULL));
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

#endif //SDPA_NRE_HPP
