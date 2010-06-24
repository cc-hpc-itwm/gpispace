/*
 * =====================================================================================
 *
 *       Filename:  SchedulerNRE.hpp
 *
 *    Description:  The aggregator's scheduler
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
#ifndef SDPA_SchedulerNRE_HPP
#define SDPA_SchedulerNRE_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/daemon/nre/messages.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
	namespace daemon {
	template <typename U>
  class SchedulerNRE : public SchedulerImpl {

  public:
		SchedulerNRE( sdpa::daemon::IComm* pHandler = NULL, std::string workerUrl = "",
				const bool bLaunchNrePcd = false,
				const char* szNrePcdBinPath = "",
				const char* szKDMModulesPath = "",
				const char* szFvmPCModule = "" ):
				sdpa::daemon::SchedulerImpl(pHandler)
				, SDPA_INIT_LOGGER((pHandler?"Scheduler "+pHandler->name():"Scheduler NRE"))
				, m_worker_(workerUrl, bLaunchNrePcd, szNrePcdBinPath, szKDMModulesPath, szFvmPCModule )
	{
		m_worker_.set_ping_interval(60);
		m_worker_.set_ping_timeout(3);
		m_worker_.set_ping_trials(3);
	}

	virtual ~SchedulerNRE() {};

	void start() throw (std::exception)
	{
		SchedulerImpl::start();

		SDPA_LOG_DEBUG("Starting NreWorkerClient ...");
		try {
			ptr_comm_handler_->rank() = m_worker_.start();
		}
		catch(const std::exception& val) {
			SDPA_LOG_ERROR("Could not start the nre-worker-client: " << val.what());
			throw;
		}
	}

	void stop()
	{
		LOG(DEBUG, "Stopping nre scheduler...");
		SchedulerImpl::stop();

		LOG(DEBUG, "Stopping nre worker...");
		m_worker_.stop();
	}

	bool post_request(bool force = false)
	{
		DMLOG(TRACE, "post request: force=" << force);
	 	bool bReqPosted = false;
	 	sdpa::util::time_type current_time = sdpa::util::now();
	 	sdpa::util::time_type difftime = current_time - m_last_request_time;

	 	if( force || ( 	difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval") &&
						ptr_comm_handler_->requestsAllowed()) )
	 	{
	 		// post a new request to the master
	 		// the slave posts a job request
	 		DMLOG(TRACE, "Post a new request to "<<ptr_comm_handler_->master());
	 		RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
	 		ptr_comm_handler_->sendEventToMaster(pEvtReq);
	 		m_last_request_time = current_time;
	 		bReqPosted = true;
	 	}
		else
		{
			DMLOG(TRACE, "not polling, difftime=" << difftime << " interval=" << ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval"));
		}

	 	return bReqPosted;
	 }

	 void send_life_sign()
	 {
	 	 sdpa::util::time_type current_time = sdpa::util::now();
	 	 sdpa::util::time_type difftime = current_time - m_last_life_sign_time;

	 	 if( ptr_comm_handler_->is_registered() )
	 	 {
	 		 if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("life-sign interval") )
	 		 {
				 DMLOG(TRACE, "sending life-sign to: " << ptr_comm_handler_->master());
	 			 LifeSignEvent::Ptr pEvtLS( new LifeSignEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
	 			 ptr_comm_handler_->sendEventToMaster(pEvtLS);
	 			 m_last_life_sign_time = current_time;
	 		 }
	 	 }
		 else
		 {
			 DMLOG(DEBUG, "not sending life-sign, i am not registered yet");
		 }
	 }

	 void check_post_request()
	 {
	 	 if( ptr_comm_handler_->is_registered() )
	 	 {
	 		 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
	 		 // post job request if number_of_jobs() < #registered workers +1
	 		 if( !post_request() )
	 			 //send a LS
	 			 send_life_sign();
	 	 }
		 else
		 {
		  DMLOG(DEBUG, "not requesting job, i am not registered yet");
		 }
	 }

	 virtual void execute(const sdpa::job_id_t& jobId) throw (std::exception)
	 {
		DLOG(TRACE, "Execute activity: " << jobId);
		const Job::ptr_t& pJob = ptr_comm_handler_->jobManager()->findJob(jobId);
		id_type act_id = pJob->id().str();

		sdpa::nre::worker::execution_result_t result;
		encoded_type enc_act = pJob->description(); // assume that the NRE's workflow engine encodes the activity!!!

		if(!ptr_comm_handler_)
		{
			SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
			result_type output_fail;
			ptr_comm_handler_->activityFailed(act_id, enc_act);
			ptr_comm_handler_->workflowEngine()->failed(act_id, output_fail);
			return;
		}

		try
		{
			ptr_comm_handler_->activityStarted(act_id, enc_act);
			DLOG(DEBUG, "walltime=" << pJob->walltime());

			result = m_worker_.execute(enc_act, pJob->walltime());
		}
		catch( const boost::thread_interrupted &)
		{
			std::string errmsg("could not execute activity: interrupted");
			SDPA_LOG_ERROR(errmsg);
			result = std::make_pair(sdpa::nre::worker::ACTIVITY_FAILED, enc_act);
		}
		catch (const std::exception &ex)
		{
			std::string errmsg("could not execute activity: ");
			errmsg += std::string(ex.what());
			SDPA_LOG_ERROR(errmsg);
			result = std::make_pair(sdpa::nre::worker::ACTIVITY_FAILED, enc_act);
		}

		// check the result state and invoke the NRE's callbacks
		if( result.first == sdpa::nre::worker::ACTIVITY_FINISHED )
		{
			SDPA_LOG_INFO("activity finished: " << act_id);
			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->activityFinished(act_id, enc_act);
			ptr_comm_handler_->workflowEngine()->finished(act_id, result.second);
		}
		else if( result.first == sdpa::nre::worker::ACTIVITY_FAILED )
		{
			SDPA_LOG_WARN("activity failed: " << act_id);
			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->activityFailed(act_id, enc_act);
			ptr_comm_handler_->workflowEngine()->failed(act_id, result.second);
		}
		else if( result.first == sdpa::nre::worker::ACTIVITY_CANCELLED )
		{
			SDPA_LOG_INFO("activity cancelled: " << act_id);

			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->activityCancelled(act_id, enc_act);
			ptr_comm_handler_->workflowEngine()->cancelled(act_id);
		}
		else
		{
			SDPA_LOG_ERROR("Invalid status of the executed activity received from the NRE worker!");
			ptr_comm_handler_->activityFailed(act_id, enc_act);
			ptr_comm_handler_->workflowEngine()->failed(act_id, result.second);
		}
	 }

	 void schedule_remote(const sdpa::job_id_t& jobId)
	 {
           throw std::runtime_error ("Schedule remote not implemented for the NREs!");
	 }

	 void run()
	 {
	 	if(!ptr_comm_handler_)
	 	{
	 		SDPA_LOG_FATAL("The scheduler cannot be started. Invalid communication handler. ");
	 		stop();
	 		return;
	 	}

	 	SDPA_LOG_DEBUG("Scheduler thread running ...");

	 	while(!bStopRequested)
	 	{
	 		try
	 		{
	 			check_post_request();
	 			sdpa::job_id_t jobId = jobs_to_be_scheduled.pop_and_wait(m_timeout);
	 			const Job::ptr_t& pJob = ptr_comm_handler_->jobManager()->findJob(jobId);

	 			if(pJob->is_local())
	 				schedule_local(jobId);
	 			else
	 			{
					try {

						SDPA_LOG_DEBUG("Try to execute the job "<<jobId.str()<<" ...");
						execute(jobId);
					}
					catch(JobNotFoundException& ex)
					{
						SDPA_LOG_DEBUG("Job not found! Could not schedule locally the job "<<ex.job_id().str());
					}
					catch(const NoWorkerFoundException&)
					{
						// put the job back into the queue
						jobs_to_be_scheduled.push(jobId);
						SDPA_LOG_DEBUG("Cannot schedule the job. No worker available! Put the job back into the queue.");
					}
	 			}
	 		}
	 		catch(JobNotFoundException& ex)
	 		{
	 			SDPA_LOG_DEBUG("Job not found! Could not schedule locally the job "<<ex.job_id().str());
	 		}
	 		catch( const boost::thread_interrupted & )
	 		{
	 			DMLOG(DEBUG, "Thread interrupted ...");
	 			bStopRequested = true; // FIXME: can probably be removed
	 			break;
	 		}
	 		catch( const sdpa::daemon::QueueEmpty &)
	 		{
	 		  // ignore
	 		}
	 		catch ( const std::exception &ex )
	 		{
	 		  MLOG(ERROR, "exception in scheduler thread: " << ex.what());
	 		}
	 	}
	 }


	 friend class boost::serialization::access;
	 friend class sdpa::tests::WorkerSerializationTest;

	 template <class Archive>
	 void serialize(Archive& ar, const unsigned int file_version )
	 {
		ar & boost::serialization::base_object<SchedulerImpl>(*this);
		//ar & m_worker_;  //NreWorkerClient
	 }

	 U& nre_worker_client() { return m_worker_;}

  private:
	  SDPA_DECLARE_LOGGER();
	  U m_worker_;
  };
}}

#endif
