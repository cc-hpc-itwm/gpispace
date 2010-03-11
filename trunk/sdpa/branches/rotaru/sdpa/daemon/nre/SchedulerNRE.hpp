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

#include <sdpa/daemon/nre/NreWorkerClient.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
	namespace daemon {
  class SchedulerNRE : public SchedulerImpl {

  public:
		SchedulerNRE( sdpa::daemon::IComm* pHandler = NULL, std::string workerUrl = ""):
				sdpa::daemon::SchedulerImpl(pHandler)
				,SDPA_INIT_LOGGER((pHandler?"Scheduler "+pHandler->name():"Scheduler NRE"))
				//,m_worker_(workerUrl)
	{
		/*m_worker_.set_ping_interval(60);
		m_worker_.set_ping_timeout(3);
		m_worker_.set_ping_trials(3);*/
	}


	 virtual ~SchedulerNRE() {};

	 bool post_request(bool force = false)
	 {
		DMLOG(TRACE, "post request: force=" << force);
	 	bool bReqPosted = false;
	 	sdpa::util::time_type current_time = sdpa::util::now();
	 	sdpa::util::time_type difftime = current_time - m_last_request_time;

	 	if(force || (difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval") ))
	 	{
	 		// post a new request to the master
	 		// the slave posts a job request
	 		SDPA_LOG_DEBUG("Post a new request to "<<ptr_comm_handler_->master());
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
				DMLOG(DEBUG, "sending life-sign to: " << ptr_comm_handler_->master());
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
	 		 if( jobs_to_be_scheduled.size() <= numberOfWorkers() + 1)
	 			 post_request();
	 		 else //send a LS
	 			 send_life_sign();
	 	 }
		 else
		 {
		  DMLOG(DEBUG, "not requesting job, i am not registered yet");
		 }
	 }

	 virtual void execute(const sdpa::job_id_t& jobId) throw (std::exception)
	 {
		SDPA_LOG_DEBUG("Execute activity ...");
		const Job::ptr_t& pJob = ptr_comm_handler_->jobManager()->findJob(jobId);
		id_type act_id = pJob->id().str();

		if(!ptr_comm_handler_)
		{
			SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
			result_type output_fail;
			ptr_comm_handler_->gwes()->finished(act_id, output_fail);
			return;
		}

		result_type output; // to be fiile-in by the NreWorkerClient
		ptr_comm_handler_->gwes()->finished(act_id, output);

		// call here the NreWorkerClient
		/*try
		{
		  //ptr_comm_handler_->activityStarted(gwes_act);
		  //result = m_worker_.execute(act, act.properties().get<unsigned long>("walltime", 0));
		}
		catch( const boost::thread_interrupted &)
		{
		  SDPA_LOG_ERROR("could not execute activity: interrupted" );
		  result.state () = sdpa::wf::Activity::ACTIVITY_FAILED;
		  result.reason() = "interrupted";
		}
		catch (const std::exception &ex)
		{
		  SDPA_LOG_ERROR("could not execute activity: " << ex.what());
		  result.state () = sdpa::wf::Activity::ACTIVITY_FAILED;
		  result.reason() = ex.what();
		}
		*/

		// check the result state and invoke the NRE's callbacks
		/*SDPA_LOG_DEBUG("Finished activity execution: notify WE ...");
		if( result.state() == FINISHED )
		{
			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->gwes()->finished(act_id, output);
		}
		else if( result.state() == FAILED )
		{
			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->gwes()->failed(act_id, output);
		}
		else if( result.state() == CANCELLED )
		{

			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->gwes()->cancelled(act_id);
		}
		else
		{
			// notify the gui
			// and then, the workflow engine
			ptr_comm_handler_->gwes()->failed(act_id, output);
		}
		*/
	 }

	 /*
	  * hand it over to the NREClient!
	  */
	 void schedule_remote(const sdpa::job_id_t& jobId)
	 {
	 	SDPA_LOG_DEBUG("Called schedule_remote ...");

	 	try {
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

  private:
	  SDPA_DECLARE_LOGGER();
	  //sdpa::nre::worker::NreWorkerClient m_worker_;
  };
}}

#endif
