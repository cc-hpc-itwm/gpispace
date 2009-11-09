#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace std;

namespace sdpa {

	class SchedulerNRE : public sdpa::daemon::SchedulerImpl
	{
	public:
		SDPA_DECLARE_LOGGER();

		SchedulerNRE(sdpa::Sdpa2Gwes* ptr_Sdpa2Gwes, sdpa::daemon::IComm* pHandler, std::string& answerStrategy):
			sdpa::daemon::SchedulerImpl(ptr_Sdpa2Gwes,  pHandler),
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

					if(pJob->is_local())
						schedule_local(pJob);
					else
						start_job(pJob);

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

		void start_job(const sdpa::daemon::Job::ptr_t &pJob)
		{
			SDPA_LOG_DEBUG("Execute job ...");

			string worker = "Scheduler";

			//first put the job into the running state
			pJob->Dispatch();

			// execute the job and ...
			// ... submit a JobFinishedEvent to the master
			if( m_answerStrategy == "finished" )
				ptr_comm_handler_->jobFinished(worker, pJob->id());
			else if( m_answerStrategy == "failed" )
				ptr_comm_handler_->jobFailed(worker, pJob->id());
			else if( m_answerStrategy == "cancelled" )
				ptr_comm_handler_->jobCancelled(worker, pJob->id());
		}


	private:
		 std::string m_answerStrategy;
	};
}
