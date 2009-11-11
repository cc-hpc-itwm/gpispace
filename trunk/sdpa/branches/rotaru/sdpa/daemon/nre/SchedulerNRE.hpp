#include <sdpa/daemon/SchedulerImpl.hpp>
#include <gwes/IActivity.h>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <sdpa/wf/GwesGlue.hpp>
using namespace std;

namespace sdpa {
	namespace daemon {

	class SchedulerNRE : public sdpa::daemon::SchedulerImpl
	{
	public:
		typedef sdpa::daemon::SynchronizedQueue<std::list<gwes::activity_t*> > ActivityQueue;

		SDPA_DECLARE_LOGGER();

		SchedulerNRE( sdpa::daemon::IComm* pHandler, std::string& answerStrategy):
			sdpa::daemon::SchedulerImpl(pHandler),
			SDPA_INIT_LOGGER("SchedulerNRE"),
			m_answerStrategy(answerStrategy)
			{}

		virtual ~SchedulerNRE() { }

		void run()
		{
			SDPA_LOG_DEBUG("Scheduler thread running ...");

			while(!bStopRequested)
			{
				try
				{
					sdpa::daemon::Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait(m_timeout);

					// schedule only jobs submitted by the master
					if(pJob->is_local())
						schedule_local(pJob);

					//check_post_request();
				}
				catch( const boost::thread_interrupted & )
				{
					SDPA_LOG_DEBUG("Thread interrupted ...");
					bStopRequested = true;
				}
				catch( const sdpa::daemon::QueueEmpty &)
				{
					//SDPA_LOG_DEBUG("Queue empty exception");
					//check_post_request();
				}
			}
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
					gwes::activity_t* pAct = activities_to_be_executed.pop_and_wait(m_timeout);
					execute(*pAct);

					SDPA_LOG_DEBUG("Finished execute_activity ...");
					check_post_request();
				}
				catch( const boost::thread_interrupted & )
				{
					SDPA_LOG_DEBUG("Thread interrupted ...");
					bStopRequested = true;
				}
				catch( const sdpa::daemon::QueueEmpty &)
				{
					//SDPA_LOG_DEBUG("Queue empty exception");
					check_post_request();
				}
			}
		}

		void execute(const gwes::activity_t& activity)
		{
			SDPA_LOG_DEBUG("Execute activity ...");

			gwes::Activity& gwes_act = (gwes::Activity&)(activity);
			sdpa::wf::Activity act = sdpa::wf::glue::wrap(gwes_act);
			act.parameters()["output"].token().data("hello");
			//alex will provide code

			sdpa::wf::glue::unwrap(act, gwes_act);

			// execute the job and ...
			sdpa::parameter_list_t output; // = *gwes_act.getTransitionOccurrence()->getTokens();

			gwes::activity_id_t act_id = activity.getID();
			gwes::workflow_id_t wf_id  = activity.getOwnerWorkflowID();

			SDPA_LOG_DEBUG("Finished activity execution: notify GWES ...");
			if( m_answerStrategy == "finished" )
				ptr_comm_handler_->gwes()->activityFinished(wf_id, act_id, output);
			else if( m_answerStrategy == "failed" )
				ptr_comm_handler_->gwes()->activityFailed(wf_id, act_id, output);
			else if( m_answerStrategy == "cancelled" )
				ptr_comm_handler_->gwes()->activityCanceled(wf_id, act_id);

		}

		void start()
		{
			SchedulerImpl::start();
			m_threadExecutor = boost::thread(boost::bind(&SchedulerNRE::runExecutor, this));
			SDPA_LOG_DEBUG("Executor thread started ...");
		}

		void stop()
		{
			SchedulerImpl::stop();
			m_threadExecutor.interrupt();

			SDPA_LOG_DEBUG("Executor thread before join ...");
			m_threadExecutor.join();
		}

	private:
		ActivityQueue activities_to_be_executed;
		std::string m_answerStrategy;
		boost::thread m_threadExecutor;
	};
}}
