/*
 * =====================================================================================
 *
 *       Filename:  SchedulerNRE.hpp
 *
 *    Description:  The NRE's scheduler
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
#include <sdpa/daemon/SchedulerImpl.hpp>
#include <gwes/IActivity.h>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <sdpa/daemon/nre/NreWorkerClient.hpp>
#include <sdpa/wf/GwesGlue.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>

using namespace sdpa::events;
using namespace sdpa::daemon;
using namespace std;

namespace sdpa {
	namespace daemon {

	class SchedulerNRE : public sdpa::daemon::SchedulerImpl
	{
	public:
		typedef sdpa::daemon::SynchronizedQueue<std::list<gwes::activity_t*> > ActivityQueue;

		SDPA_DECLARE_LOGGER();

		SchedulerNRE( sdpa::daemon::IComm* pHandler, std::string workerUrl ):
			sdpa::daemon::SchedulerImpl(pHandler),
			SDPA_INIT_LOGGER("Scheduler "+pHandler->name()),
	        m_worker_(workerUrl)
			{
				m_worker_.set_ping_interval(60);
				m_worker_.set_ping_timeout(3);
				m_worker_.set_ping_trials(3);
			}

		virtual ~SchedulerNRE()
        {
          try
          {
            stop();
          }
          catch (const std::exception &ex)
          {
            LOG(ERROR, "could not stop nre-scheduler: " << ex.what());
          }
          catch (...)
          {
            LOG(ERROR, "could not stop nre-scheduler (unknown error)");
          }
        }

		void run()
		{
			SDPA_LOG_DEBUG("Scheduler thread running ...");

			while(!bStopRequested)
			{
				try
				{
					check_post_request();
					sdpa::daemon::Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait(m_timeout);

					// schedule only jobs submitted by the master
					if(pJob->is_local())
					{
						post_request(true);
						schedule_local(pJob);
					}
				}
				catch( const boost::thread_interrupted & )
				{
					SDPA_LOG_DEBUG("Thread interrupted ...");
					bStopRequested = true;
				}
				catch( const sdpa::daemon::QueueEmpty &)
				{
				}
				catch (const std::exception &ex)
				{
				  SDPA_LOG_ERROR("unexpected exception in scheduler thread: " << ex.what());
				}
			}
		}

        void schedule(Job::ptr_t & pJob)
        {
          return SchedulerImpl::schedule(pJob);
        }

		void schedule(gwes::activity_t& act)
		{
			SDPA_LOG_DEBUG("Handle activity "<<act.getID());
			activities_to_be_executed.push(&act);
		}

		void runExecutor()
		{
			SDPA_LOG_DEBUG("Executor thread running ...");

			while(!bStopRequested)
			{
				try
				{
					gwes::activity_t *pAct = activities_to_be_executed.pop_and_wait(m_timeout);
					DMLOG(TRACE, "prefetching next activity...");
					post_request(true); // prefetch

					execute(*pAct);

					SDPA_LOG_DEBUG("Finished executing the activity activity "<<pAct->getID());
					post_request(true); // check for more work
				}
				catch( const boost::thread_interrupted & )
				{
					SDPA_LOG_DEBUG("Thread interrupted ...");
					bStopRequested = true;
				}
				catch( const sdpa::daemon::QueueEmpty &)
				{
				  // ignore
				}
				catch (const std::exception &ex )
				{
				  SDPA_LOG_ERROR("unexpected exception during activity execution: " << ex.what());
				}
			}
		}

		void execute(const gwes::activity_t& activity) throw (std::exception)
		{
			SDPA_LOG_DEBUG("Execute activity ...");

			gwes::Activity& gwes_act = (gwes::Activity&)(activity);
			sdpa::wf::Activity result;
			try
			{
			  sdpa::wf::Activity act(sdpa::wf::glue::wrap(gwes_act));

			  ptr_comm_handler_->activityStarted(gwes_act);
			  result = m_worker_.execute(act, act.properties().get<unsigned long>("walltime", 0));

			  sdpa::wf::glue::unwrap(result, gwes_act);
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

			sdpa::parameter_list_t output = *gwes_act.getTransitionOccurrence()->getTokens();

			gwes::activity_id_t act_id = activity.getID();
			gwes::workflow_id_t wf_id  = activity.getOwnerWorkflowID();

			SDPA_LOG_DEBUG("Finished activity execution: notify GWES ...");
			if( result.state() == sdpa::wf::Activity::ACTIVITY_FINISHED )
			{
				ptr_comm_handler_->activityFinished(gwes_act);
				ptr_comm_handler_->gwes()->activityFinished(wf_id, act_id, output);
			}
			else if( result.state() == sdpa::wf::Activity::ACTIVITY_FAILED )
			{
				ptr_comm_handler_->activityFailed(gwes_act);
				ptr_comm_handler_->gwes()->activityFailed(wf_id, act_id, output);
			}
			else if( result.state() == sdpa::wf::Activity::ACTIVITY_CANCELED )
			{
				ptr_comm_handler_->activityCancelled(gwes_act);
				ptr_comm_handler_->gwes()->activityCanceled(wf_id, act_id);
			}
			else
			{
				ptr_comm_handler_->activityFailed(gwes_act);
				ptr_comm_handler_->gwes()->activityFailed(wf_id, act_id, output);
			}
		}

		void start()throw (std::exception)
		{
			SchedulerImpl::start();
			m_threadExecutor = boost::thread(boost::bind(&SchedulerNRE::runExecutor, this));
			SDPA_LOG_DEBUG("Executor thread started ...");

			SDPA_LOG_DEBUG("Starting nre-worker-client ...");

			try {
				m_worker_.start();
			}catch(const std::exception& val)
			{
				SDPA_LOG_ERROR("Could not start the nre-worker-client ...");
				throw;
			}

		}

		void stop()
		{
			SchedulerImpl::stop();
			m_threadExecutor.interrupt();

			SDPA_LOG_DEBUG("Executor thread before join ...");
			m_threadExecutor.join();

			SDPA_LOG_DEBUG("Stopping nre-worker-client ...");
			m_worker_.stop();
		}

		 bool post_request(bool force=false)
		 {
			DMLOG(TRACE, "post request: force=" << force);
			bool bReqPosted = false;
			sdpa::util::time_type current_time = sdpa::util::now();
			sdpa::util::time_type difftime = current_time - m_last_request_time;

			if( ptr_comm_handler_->is_registered() )
			{
				if(force || (difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval")))
				{
					DMLOG(TRACE, "polling, difftime=" << difftime << " interval=" << ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval"));
					// post a new request to the master
					// the slave posts a job request
					SDPA_LOG_DEBUG("Post a new request to "<<ptr_comm_handler_->master());
					RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
					ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pEvtReq);
					m_last_request_time = current_time;
					bReqPosted = true;
				}
			  else
			  {
				DMLOG(TRACE, "not polling, difftime=" << difftime << " interval=" << ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval"));
			  }
		  }
		 else
		 {
		  DMLOG(DEBUG, "not posting request, i am not registered yet");
		 }

			return bReqPosted;
		 }

		 void send_life_sign()
		 {
			 sdpa::util::time_type current_time = sdpa::util::now();
			 sdpa::util::time_type difftime = current_time - m_last_life_sign_time;

			 if( ptr_comm_handler_->is_registered() )
			 {
				DMLOG(TRACE, "sending life-sign, difftime=" << difftime << " interval=" << ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("life-sign interval"));
  				 if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("life-sign interval") )
				 {
					 LifeSignEvent::Ptr pEvtLS( new LifeSignEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
					 ptr_comm_handler_->sendEvent(ptr_comm_handler_->to_master_stage(), pEvtLS);
					 m_last_life_sign_time = current_time;
				 }
			 }
		 }

		 void check_post_request()
		 {
			 if( ptr_comm_handler_->is_registered() )
			 {
				 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
				 // post job request if number_of_jobs() < #registered workers +1
				 if( jobs_to_be_scheduled.size() <= 2)
					 post_request();
				 else //send a LS
					 send_life_sign();
			 }
		 }

	private:
		ActivityQueue activities_to_be_executed;
		boost::thread m_threadExecutor;
		sdpa::nre::worker::NreWorkerClient m_worker_;
	};
}}
