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
			: DaemonFSM( name, dynamic_cast<IWorkflowEngine*>(new T(this) )),
				  SDPA_INIT_LOGGER(name),
				  url_(url),
				  masterName_(masterName),
				  masterUrl_(masterUrl)
			{
				SDPA_LOG_DEBUG("Aggregator constructor called ...");
				ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new sdpa::daemon::SchedulerAgg(this));
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
void Aggregator<T>::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_configure'");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put<sdpa::util::time_type>("polling interval", 1000000); //1s
	ptr_daemon_cfg_->put<sdpa::util::time_type>("life-sign interval", 1000000); //1s
}

template <typename T>
void Aggregator<T>::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");
	// in fact the master name should be red from the configuration file

	SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
	WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
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

	if( pEvt->from() == sdpa::daemon::WE ) // use a predefined variable here of type enum or use typeid
	{
		try {
			// forward it up
			JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), master(), pEvt->job_id(), pEvt->result()));

			// send the event to the master
			sendEventToMaster(pEvtJobFinished, MSG_RETRY_CNT);
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
			if(ptr_workflow_engine_)
			{
				id_type actId = pJob->id().str();

				SDPA_LOG_DEBUG("Inform WE that the activity "<<actId<<" finished");
				result_type output = pEvt->result();
				ptr_workflow_engine_->failed(actId, output);

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

template <typename T>
void Aggregator<T>::handleCancelJobEvent(const CancelJobEvent* pEvt )
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

template <typename T>
void Aggregator<T>::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
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
    	if( pEvt->from() == sdpa::daemon::WE  ) // the message comes from GWES, forward it to the master
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
    		id_type actId = pJob->id();

    		// inform gwes that the activity was canceled
    		ptr_workflow_engine_->cancelled(actId);
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
