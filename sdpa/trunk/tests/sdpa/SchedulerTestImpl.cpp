#include "SchedulerTestImpl.hpp"

using namespace sdpa::tests;
using namespace std;

SchedulerTestImpl::SchedulerTestImpl():
	SDPA_INIT_LOGGER("sdpa::tests::SchedulerTestImpl")
{
}

SchedulerTestImpl::~SchedulerTestImpl()
{
	SDPA_LOG_DEBUG("Called the destructor of  SchedulerTestImpl ...");
}

void SchedulerTestImpl::schedule(sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("Handle job "<<jobId.str());
	jobs_to_be_scheduled.push(jobId);
}

bool SchedulerTestImpl::schedule_to(const sdpa::job_id_t& /*jobId*/, const sdpa::worker_id_t& /*rank*/ )
{
	return false;
}

void SchedulerTestImpl::re_schedule(Worker::JobQueue* /*pQueue*/ )
{

}


const Worker::ptr_t& SchedulerTestImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	throw WorkerNotFoundException(worker_id);
}

const Worker::worker_id_t& SchedulerTestImpl::findWorker(const sdpa::job_id_t& /*job_id*/) throw (NoWorkerFoundException)
{
	throw NoWorkerFoundException();
}

void SchedulerTestImpl::addWorker( const Worker::worker_id_t& /*workerId*/, unsigned int /*rank*/ ) throw (WorkerAlreadyExistException)
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
			//Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait();
			sdpa::job_id_t jobId = jobs_to_be_scheduled.pop_and_wait();

			if( jobId != sdpa::job_id_t::invalid_job_id() )
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

void SchedulerTestImpl::print()
{
	jobs_to_be_scheduled.print();
	ptr_worker_man_->print();
}

const sdpa::job_id_t SchedulerTestImpl::getNextJob(const Worker::worker_id_t& /*worker_id*/, const sdpa::job_id_t & /*last_job_id*/) throw (NoJobScheduledException)
{
	sdpa::job_id_t jobId;
	return jobId;
}

void SchedulerTestImpl::acknowledgeJob(const Worker::worker_id_t& /*worker_id*/, const sdpa::job_id_t& /*job_id*/) throw(WorkerNotFoundException, JobNotFoundException)
{
}

void SchedulerTestImpl::deleteWorkerJob(const Worker::worker_id_t& /*worker_id*/, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException)
{
	throw JobNotDeletedException(job_id);
}
