/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
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
#ifndef SDPA_ORCHESTRATORTOR_HPP
#define SDPA_ORCHESTRATORTOR_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/orchestrator/SchedulerOrch.hpp>
#include <tests/sdpa/test_Scheduler.hpp>

namespace sdpa {
namespace daemon {
      template <typename T>
	  class Orchestrator : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<Orchestrator<T> > ptr_t;
		SDPA_DECLARE_LOGGER();

		Orchestrator(  	const std::string &name = "",
						const std::string& url = "",
                        const std::string &/*workflow_directory*/ = "",
                        const bool& bHasWe = true)
		: DaemonFSM( name, bHasWe?new T(this, boost::bind(&GenericDaemon::gen_id, this) ):NULL ),
			  SDPA_INIT_LOGGER(name),
			  url_(url)
		{
			SDPA_LOG_DEBUG("Orchestrator constructor called ...");

			//ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerOrch(this));
		}

		static ptr_t create( const std::string& name,
							 const std::string& url,
							 const std::string &workflow_directory )
		{
			return ptr_t(new Orchestrator<T>( name, url, workflow_directory ));
		}

		virtual ~Orchestrator();

		static void start(ptr_t ptrOrch);
		static void shutdown(ptr_t ptrOrch);

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );
		void action_interrupt( const sdpa::events::InterruptEvent& );

		void handleJobFinishedEvent( const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent( const sdpa::events::JobFailedEvent* );

		void handleCancelJobEvent( const sdpa::events::CancelJobEvent* pEvt );
		void handleCancelJobAckEvent( const sdpa::events::CancelJobAckEvent* pEvt );

		void handleRetrieveJobResultsEvent( const sdpa::events::RetrieveJobResultsEvent* pEvt );

		const std::string& url() const {return url_;}

		template <class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar & boost::serialization::base_object<DaemonFSM>(*this);
			ar & url_; //boost::serialization::make_nvp("url_", url_);
		}

		virtual void backup( const std::string& strArchiveName );
		virtual void recover( const std::string& strArchiveName );

		friend class boost::serialization::access;
		friend class sdpa::tests::WorkerSerializationTest;
		friend class sdpa::tests::SchedulerTest;

	  private:
		Scheduler* create_scheduler()
		{
            DLOG(TRACE, "creating orchestrator scheduler...");
			return new SchedulerOrch(this);
		}

		std::string url_;
	  };
	}
}

// Implementation
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;


template <typename T>
Orchestrator<T>::~Orchestrator()
{
	SDPA_LOG_DEBUG("Orchestrator destructor called ...");
	daemon_stage_ = NULL;
}


template <typename T>
void Orchestrator<T>::start( Orchestrator<T>::ptr_t ptrOrch )
{
	dsm::DaemonFSM::create_daemon_stage(ptrOrch);
	ptrOrch->configure_network( ptrOrch->url() );
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrOrch, ptrCfg);
}

template <typename T>
void Orchestrator<T>::shutdown( Orchestrator<T>::ptr_t ptrOrch )
{
	ptrOrch->shutdown_network();
	ptrOrch->stop();

	delete ptrOrch->ptr_workflow_engine_;
	ptrOrch->ptr_workflow_engine_ = NULL;
}

template <typename T>
void Orchestrator<T>::action_configure(const StartUpEvent &se)
{
	GenericDaemon::action_configure (se);

	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuring myeself (orchestrator)...");
}

template <typename T>
void Orchestrator<T>::action_config_ok(const ConfigOkEvent& e)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuration (orchestrator) was ok");
	{
	  std::ostringstream sstr;
	  ptr_daemon_cfg_->writeTo (sstr);
	  SDPA_LOG_INFO("config: " << sstr.str());
	}

        GenericDaemon::action_config_ok (e);
}

template <typename T>
void Orchestrator<T>::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

}

template <typename T>
void Orchestrator<T>::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
	assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	DLOG(TRACE, "handleJobFinished(" << pEvt->job_id() << ")");

	if (pEvt->from() != sdpa::daemon::WE)
	{
		// send a JobFinishedAckEvent back to the worker/slave
		JobFinishedAckEvent::Ptr evt(new JobFinishedAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

		// send the event to the slave
		sendEventToSlave(evt);
	}

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished(pEvt);

	}
	catch(JobNotFoundException const &)
	{
          SDPA_LOG_WARN( "got finished message for old/unknown Job "<< pEvt->job_id());
          return;
	}

	// It's an worker who sent the message
	if( (pEvt->from() != sdpa::daemon::WE) )
	{
		Worker::worker_id_t worker_id = pEvt->from();
        id_type act_id = pEvt->job_id();

		try {
			SDPA_LOG_DEBUG("Inform WE that the activity "<<act_id<<" finished");
            result_type output = pEvt->result();
            ptr_workflow_engine_->finished(act_id, output);

            try {
            	ptr_scheduler_->deleteWorkerJob ( worker_id, act_id );
			}
			catch(WorkerNotFoundException const &)
			{
				SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
			}
			catch(const JobNotDeletedException& ex)
			{
				SDPA_LOG_WARN( "Could not delete the job "
						 << act_id
						 << " from worker "
						 << worker_id
						 << "queues: "
						 << ex.what()
						 );
			}

			try {
				//delete it also from job_map_
				ptr_job_man_->deleteJob(act_id);
			}
			catch(JobNotDeletedException const &)
			{
				SDPA_LOG_WARN("The JobManager could not delete the job "<< act_id);
			}

		}catch(...) {
			SDPA_LOG_ERROR("Unexpected exception occurred!");
		}
	}
}

template <typename T>
void Orchestrator<T>::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
        assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

        DLOG(TRACE, "handleJobFailed(" << pEvt->job_id() << ")");

        if (pEvt->from() != sdpa::daemon::WE)
        {
          // send a JobFinishedAckEvent back to the worker/slave
          JobFailedAckEvent::Ptr evt
            (new JobFailedAckEvent ( name()
                                   , pEvt->from()
                                   , pEvt->job_id()
                                   , pEvt->id()
                                   )
            );
          // send the event to the slave
          sendEventToSlave(evt);
        }

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFailed(pEvt);
	}
	catch(const JobNotFoundException &){
		SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
                return;
	}

	if(pEvt->from() == sdpa::daemon::WE )
	{
		pJob->setResult(pEvt->result());
	}
	else
	{
		Worker::worker_id_t worker_id = pEvt->from();
                id_type actId = pJob->id().str();

		try {
                  SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" failed");
                  result_type output = pEvt->result();
                  ptr_workflow_engine_->failed(actId, output);

                  try {
                    ptr_scheduler_->deleteWorkerJob(worker_id, pJob->id());
                  }
                  catch(const WorkerNotFoundException&)
                  {
                    SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
                  }
                  catch(const JobNotDeletedException&)
                  {
                    SDPA_LOG_WARN("Could not delete the job "<<pJob->id()<<" from the "<<worker_id<<"'s queues ...");
                  }

                  try {
                    //delete it also from job_map_
                    ptr_job_man_->deleteJob(pEvt->job_id());
                  }
                  catch(const JobNotDeletedException&)
                  {
                    SDPA_LOG_WARN("The JobManager could not delete the job "<<pJob->id());
                  }
		}
		catch(...) {
                  SDPA_LOG_ERROR("Unexpected exception occurred!");
		}
	}
}

template <typename T>
void Orchestrator<T>::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  assert (pEvt);

  DLOG(TRACE, "handleCancelJob(" << pEvt->job_id() << ")");

	try {
                Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->CancelJob(pEvt);
		SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

		CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id(), pEvt->id()));

		// only if the job was already submitted
		sendEventToMaster(pCancelAckEvt);
	}
	catch(JobNotFoundException const &){
          SDPA_LOG_WARN("could not find job: " << pEvt->job_id());
	}
}

template <typename T>
void Orchestrator<T>::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
        assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

        DLOG(TRACE, "handleCancelJobAck(" << pEvt->job_id() << ")");

	// transition from Cancelling to Cancelled

	Worker::worker_id_t worker_id = pEvt->from();
	Job::ptr_t pJob;
	try {
          pJob = ptr_job_man_->findJob(pEvt->job_id());

          // put the job into the state Cancelled
          pJob->CancelJobAck(pEvt);
          SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());

          if( pEvt->from() != sdpa::daemon::WE  )
          {
            try {
              ptr_scheduler_->deleteWorkerJob(worker_id, pJob->id());
            }
            catch(const WorkerNotFoundException&) {
              SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
            }
            catch(const JobNotDeletedException&)
            {
              SDPA_LOG_ERROR("Could not delete the job "<<pJob->id()<<" from the worke "<<worker_id<<"'s queues ...");
            }

            // inform WE that the activity was canceled
            ptr_workflow_engine_->cancelled(pEvt->job_id());
          }
	}
	catch(const JobNotFoundException&)
	{
		SDPA_LOG_ERROR("could not find job " << pEvt->job_id());
	}
	catch(const JobNotDeletedException& ex)
	{
		SDPA_LOG_ERROR("could not delete job: " << ex.what());
	}
	catch(...) {
		SDPA_LOG_FATAL("an unexpected exception occured during job-deletion!");
	}
}

template <typename T>
void Orchestrator<T>::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* pEvt )
{
	try {
		Job::ptr_t pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->RetrieveJobResults(pEvt);
	}
	catch(const JobNotFoundException&)
	{
		SDPA_LOG_INFO("The job "<<pEvt->job_id()<<" was not found by the JobManager");
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, "no such job") );
		sendEventToMaster(pErrorEvt);
	}
}

template <typename T>
void Orchestrator<T>::backup( const std::string& strArchiveName )
{
	try
	{
		//print();

		Orchestrator* ptrOrch(this);
		std::ofstream ofs( strArchiveName.c_str() );
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<Orchestrator<T>*>(NULL));
		oa.register_type(static_cast<T*>(NULL));
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerOrch*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		oa << ptrOrch;

		SDPA_LOG_DEBUG("Successfully serialized the Orchestrator into "<<strArchiveName);
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: "<< e.what() << endl;
		return ;
	}
}

template <typename T>
void Orchestrator<T>::recover( const std::string& strArchiveName )
{
	try
	{
		Orchestrator* pRestoredOrch(this);
		std::ifstream ifs( strArchiveName.c_str() );
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Orchestrator<T>*>(NULL));
		ia.register_type(static_cast<T*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerOrch*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		ia >> pRestoredOrch;

		SDPA_LOG_DEBUG("Successfully de-serialized the Orchestrator from "<<strArchiveName);
		pRestoredOrch->print();
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: " << e.what() << endl;
		return;
	}
}

#endif //SDPA_ORCHESTRATORTOR_HPP
