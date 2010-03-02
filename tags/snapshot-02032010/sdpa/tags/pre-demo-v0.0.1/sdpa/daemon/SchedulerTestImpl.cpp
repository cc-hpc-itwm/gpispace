#include "SchedulerTestImpl.hpp"

using namespace std;

using namespace sdpa::daemon;

SchedulerTestImpl::SchedulerTestImpl(sdpa::Sdpa2Gwes* /* ptr_Sdpa2Gwes */):
	SDPA_INIT_LOGGER("sdpa::daemon::SchedulerTestImpl")
{
}

SchedulerTestImpl::~SchedulerTestImpl()
{
	SDPA_LOG_DEBUG("Called the destructor of  SchedulerTestImpl ...");
}

void SchedulerTestImpl::schedule(Job::ptr_t& pJob)
{
	SDPA_LOG_DEBUG("Handle job "<<pJob->id());

	jobs_to_be_scheduled.push(pJob);
}

Worker::ptr_t& SchedulerTestImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	throw WorkerNotFoundException(worker_id);
}

void SchedulerTestImpl::addWorker(const Worker::ptr_t& /* worker */)
{
}


void SchedulerTestImpl::start()
{
	bStopRequested = false;
	m_thread = boost::thread(boost::bind(&SchedulerTestImpl::run, this));
	SDPA_LOG_DEBUG("Scheduler thread started ...");
}

void SchedulerTestImpl::stop()
{
	m_thread.interrupt();
	//jobs_to_be_scheduled.stop();

	bStopRequested = true;
	SDPA_LOG_DEBUG("Scheduler thread before join ...");
	m_thread.join();
	SDPA_LOG_DEBUG("Scheduler thread joined ...");
}

void SchedulerTestImpl::run()
{
	SDPA_LOG_DEBUG("Scheduler thread running ...");

	while(!bStopRequested)
	{
		try {
			Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait();

			if( pJob.use_count() )
			{
				SDPA_LOG_DEBUG("Job scheduled!");
			}
			else
			{
				bStopRequested = true;
				SDPA_LOG_DEBUG("Thread stops now!");
			}
		}
		catch( boost::thread_interrupted )
		{
			SDPA_LOG_DEBUG("Thread interrupted ...");
			bStopRequested = true;
		}
		catch( sdpa::daemon::QueueEmpty )
		{
			SDPA_LOG_DEBUG("Queue empty exception");
			bStopRequested = true;
		}
	}
}
