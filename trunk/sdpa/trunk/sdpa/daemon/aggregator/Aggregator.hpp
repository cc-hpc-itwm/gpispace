/*
 * =====================================================================================
 *
 *       Filename:  Aggreagtor.hpp
 *
 *    Description:  Contains the Aggregator class
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
#ifndef SDPA_AGGREGATOR_HPP
#define SDPA_AGGREGATOR_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/aggregator/SchedulerAgg.hpp>

#include <boost/serialization/base_object.hpp>

namespace sdpa {
	namespace daemon {
		template <typename T>
		class Aggregator : public dsm::DaemonFSM {
			public:
			typedef sdpa::shared_ptr<Aggregator<T> > ptr_t;
			SDPA_DECLARE_LOGGER();

			Aggregator( const std::string& name = "", const std::string& url = "",
						const std::string& masterName = "", const std::string& masterUrl = "")
			: DaemonFSM( name, create_workflow_engine<T>() ),
				  SDPA_INIT_LOGGER(name),
				  url_(url),
				  masterName_(masterName),
				  masterUrl_(masterUrl)
			{
				SDPA_LOG_DEBUG("Aggregator constructor called ...");
				//ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new sdpa::daemon::SchedulerAgg(this));
			}


			virtual ~Aggregator();

			static ptr_t create( const std::string& name,
							     const std::string& url,
								 const std::string& masterName,
								 const std::string& masterUrl )
			{
				 return ptr_t( new Aggregator<T>( name, url, masterName, masterUrl));
			}

			static void start(ptr_t ptrAgg);
			static void shutdown(ptr_t ptrAgg);

			void action_configure( const sdpa::events::StartUpEvent& );
			void action_config_ok( const sdpa::events::ConfigOkEvent& );
			void action_interrupt( const sdpa::events::InterruptEvent& );

			void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
			void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

			void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
			void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

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
			}

			virtual void backup( const std::string& );
			virtual void recover( const std::string& );

			friend class boost::serialization::access;
			friend class sdpa::tests::WorkerSerializationTest;

			private:
			Scheduler* create_scheduler()
			{
				return new SchedulerAgg(this);
			}

			std::string url_;
			std::string masterName_;
			std::string masterUrl_;
		};
	}
}

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
Aggregator<T>::~Aggregator()
{
	SDPA_LOG_DEBUG("Aggregator destructor called ...");
	daemon_stage_ = NULL;
}

template <typename T>
void Aggregator<T>::start(Aggregator<T>::ptr_t ptrAgg)
{
	dsm::DaemonFSM::create_daemon_stage(ptrAgg);
	ptrAgg->configure_network( ptrAgg->url(), ptrAgg->masterName(), ptrAgg->masterUrl());
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrAgg, ptrCfg);
}

template <typename T>
void Aggregator<T>::shutdown(Aggregator<T>::ptr_t ptrAgg)
{
	ptrAgg->shutdown_network();
	ptrAgg->stop();

	delete ptrAgg->ptr_workflow_engine_;
	ptrAgg->ptr_workflow_engine_ = NULL;
}

template <typename T>
void Aggregator<T>::action_configure(const StartUpEvent &se)
{
	GenericDaemon::action_configure (se);

	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuring myeself (aggregator)...");
}

template <typename T>
void Aggregator<T>::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuration (aggregator) was ok");
	{
	  std::ostringstream sstr;
	  ptr_daemon_cfg_->writeTo (sstr);
	  SDPA_LOG_INFO("config: " << sstr.str());
	}

	SDPA_LOG_INFO("Aggregator (" << name() << ") sending registration event to master (" << master() << ")");
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master(), rank()));
	to_master_stage()->send(pEvtWorkerReg);
}

template <typename T>
void Aggregator<T>::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state

}

template <typename T>
void Aggregator<T>::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
        assert (pEvt);

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

        DLOG(TRACE, "job finished: " << pEvt->job_id());

	//put the job into the state Finished
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->JobFinished(pEvt);
	}
	catch(JobNotFoundException){
		SDPA_LOG_ERROR("Job "<<pEvt->job_id()<<" not found!");
                return;
	}

	if( pEvt->from() == sdpa::daemon::WE ) // use a predefined variable here of type enum or use typeid
	{
		try {
			// forward it up
			JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEventToMaster(pEvtJobFinished, MSG_RETRY_CNT);
			// delete it from the map when you receive a JobFaileddAckEvent!
		}
		catch(QueueFull const &)
		{
			SDPA_LOG_ERROR("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFinishedEvent");
		}
		catch(seda::StageNotFound const &)
		{
			SDPA_LOG_ERROR("Stage not found when trying to submit JobFinishedEvent");
		}
		catch(std::exception const & ex) {
                  SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
                  throw;
		}
		catch(...) {
                  SDPA_LOG_FATAL("Unexpected exception occurred!");
                  throw;
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
			if(ptr_workflow_engine_)
			{
				id_type actId = pJob->id().str();

				SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" finished");
				result_type output = pEvt->result();
				ptr_workflow_engine_->finished(actId, output);

				try {
					Worker::ptr_t ptrWorker = findWorker(worker_id);
					// delete job from the worker's queues

					SDPA_LOG_DEBUG("Delete the job "<<pEvt->job_id()<<" from the worker's queues!");
					ptrWorker->delete_job(pEvt->job_id());
				}
				catch(WorkerNotFoundException)
				{
					SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
                                        throw;
				}

				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}
				catch(JobNotDeletedException&)
				{
					SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
                                        throw;
				}
			}
			else
                        {
                          SDPA_LOG_FATAL("workflow engine has not been initialized!");
                          throw std::runtime_error ("aggregator: workflow engine not initialized!");
                        }
                }
                catch(std::exception const & ex) {
                  SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
		}
                catch(...)
                {
                  SDPA_LOG_FATAL("Unexpected exception occurred!");
                  throw;
                }
        }
}

template <typename T>
void Aggregator<T>::handleJobFailedEvent(const JobFailedEvent* pEvt )
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

	if( pEvt->from() == sdpa::daemon::WE ) // use a predefined variable here of type enum or use typeid
	{
		// the message comes from GWES
		try {
			// forward it up
			JobFailedEvent::Ptr pEvtJobFailedEvent(new JobFailedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEventToMaster(pEvtJobFailedEvent, MSG_RETRY_CNT);
			// delete it from the map when you receive a JobFaileddAckEvent!
		}
		catch(QueueFull const &)
		{
			SDPA_LOG_ERROR("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a JobFailedEvent");
		}
		catch(seda::StageNotFound const &)
		{
			SDPA_LOG_ERROR("Stage not found when trying to submit JobFailedEvent");
		}
		catch(std::exception const & ex) {
                        SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
                        throw;
		}
		catch(...) {
			SDPA_LOG_ERROR("Unexpected exception occurred!");
                        throw;
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
			if(ptr_workflow_engine_)
			{
				id_type actId = pJob->id().str();

				SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" finished");
				result_type output = pEvt->result();
				ptr_workflow_engine_->failed(actId, output);

				try {
					Worker::ptr_t ptrWorker = findWorker(worker_id);
					// delete job from worker's queues

					DLOG(TRACE, "Deleting the job " << pEvt->job_id() << " from the worker's queues!");
					ptrWorker->delete_job(pEvt->job_id());

				} catch(WorkerNotFoundException const &) {
					SDPA_LOG_ERROR("Worker "<<worker_id<<" not found!");
				}

				try {
					//delete it also from job_map_
					ptr_job_man_->deleteJob(pEvt->job_id());
				}catch(JobNotDeletedException const &){
					SDPA_LOG_ERROR("The JobManager could not delete the job "<<pEvt->job_id());
				}
			}
			else
                        {
                          SDPA_LOG_ERROR("workflow engine not initialized!");
                          throw std::runtime_error ("workflow engine has not been initialized");
                        }
		}
		catch(std::exception const & ex) {
                  SDPA_LOG_ERROR("Unexpected exception occurred: " << ex.what());
                  throw;
		}
                catch(...) {
                  SDPA_LOG_ERROR("Unexpected exception occurred!");
                  throw;
		}
	}
}

template <typename T>
void Aggregator<T>::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  LOG(INFO, "cancelling job: " << pEvt->job_id());

	Job::ptr_t pJob;
	// put the job into the state Cancelling
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());
		pJob->CancelJob(pEvt);
                DLOG(TRACE, "The job state is: "<<pJob->getStatus());
	}
	catch(JobNotFoundException const &){
          SDPA_LOG_ERROR ("job " << pEvt->job_id() << " could not be found!");
          throw;
	}
}

template <typename T>
void Aggregator<T>::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
  assert (pEvt);

  LOG(TRACE, "cancel acknowledgement received: " << pEvt->job_id());

	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job

	// transition from Cancelling to Cancelled

	Worker::worker_id_t worker_id = pEvt->from();
	Job::ptr_t pJob;
	try {
		pJob = ptr_job_man_->findJob(pEvt->job_id());

		// put the job into the state Cancelled
	    pJob->CancelJobAck(pEvt);
	    DLOG(TRACE, "The job state is: "<<pJob->getStatus());

    	// should send acknowlwdgement
    	if( pEvt->from() == sdpa::daemon::WE  ) // the message comes from GWES, forward it to the master
		{
			CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), master(), pEvt->job_id(), pEvt->id()));

			// only if the job was already submitted, send ack to master
			sendEventToMaster(pCancelAckEvt);

			// if I'm not the orchestrator delete effectively the job
			ptr_job_man_->deleteJob(pEvt->job_id());

		}
    	else // the message comes from an worker, forward it to workflow engine
    	{
    		try {
    			Worker::ptr_t ptrWorker = findWorker(worker_id);

				// the message comes from a worker
				ptrWorker->delete_job(pEvt->job_id());
    		 }
    		 catch(WorkerNotFoundException const &)
    		 {
                   SDPA_LOG_ERROR("worker " << worker_id << " could not be found!");
    		}

    		// tell WE that the activity was cancelled
    		id_type actId = pJob->id();
    		ptr_workflow_engine_->cancelled(actId);
    	}
	}
	catch(JobNotFoundException const &)
	{
		SDPA_LOG_ERROR("job could not be found: " << pEvt->job_id());
	}
	catch(JobNotDeletedException const &)
	{
		SDPA_LOG_ERROR("Job Manager could not delete job: " << pEvt->job_id());
	}
	catch (std::exception const &ex)
        {
          SDPA_LOG_ERROR("unexpected exception during cancel of: " << pEvt->job_id() << ": " << ex.what());
	}
	catch(...)
        {
          SDPA_LOG_ERROR("unexpected exception during cancel of: " << pEvt->job_id());
	}
}

template <typename T>
void Aggregator<T>::backup( const std::string& strArchiveName )
{
	try
	{
		print();

		Aggregator* ptrOrch(this);
		std::ofstream ofs( strArchiveName.c_str() );
		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<Aggregator*>(NULL));
		oa.register_type(static_cast<DaemonFSM*>(NULL));
		oa.register_type(static_cast<GenericDaemon*>(NULL));
		oa.register_type(static_cast<SchedulerImpl*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		oa << ptrOrch;
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: "<< e.what() << endl;
		return;
	}
}

template <typename T>
void Aggregator<T>::recover( const std::string& strArchiveName )
{
	try
	{
		Aggregator* pRestoredOrch(this);
		std::ifstream ifs( strArchiveName.c_str() );
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<Aggregator*>(NULL));
		ia.register_type(static_cast<DaemonFSM*>(NULL));
		ia.register_type(static_cast<GenericDaemon*>(NULL));
		ia.register_type(static_cast<SchedulerImpl*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		ia >> pRestoredOrch;

		print();
	}
	catch(exception &e)
	{
		cout <<"Exception occurred: " << e.what() << endl;
	}
}


#endif
