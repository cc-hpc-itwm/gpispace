#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace sdpa::daemon;

SchedulerImpl::SchedulerImpl(sdpa::wf::Sdpa2Gwes*  ptr_Sdpa2Gwes):
	ptr_worker_man_(new WorkerManager()),
	SDPA_INIT_LOGGER("sdpa::daemon::SchedulerImpl")
{
	if(ptr_Sdpa2Gwes)
		ptr_Sdpa2Gwes_ = ptr_Sdpa2Gwes;
	else
		ptr_Sdpa2Gwes_=NULL;
}

SchedulerImpl::~SchedulerImpl()
{

}


void SchedulerImpl::acknowledge(const sdpa::job_id_t& job_id ) {

}

Job::ptr_t SchedulerImpl::get_next_job(const Worker::worker_id_t &worker_id, const sdpa::job_id_t &last_job) {

}

/*
	Schedule a job locally, send the job to GWES
*/
void SchedulerImpl::schedule_local(const Job::ptr_t &pJob) {
	SDPA_LOG_DEBUG("Called schedule_local ...");

	sdpa::wf::workflow_t wf_desc("");//pJob->description());
	sdpa::wf::workflow_id_t wf_id = pJob->id().str();

	// Should set the workflow_id here, or send it together with the workflow description
	ostringstream os;
	os<<"Submit the workflow attached to the job "<<wf_id<<" to GWES";
	SDPA_LOG_DEBUG(os.str());

	if(ptr_Sdpa2Gwes_)
	{
		ptr_Sdpa2Gwes_->submitWorkflow(wf_id, wf_desc);

		// Only with the SMC variant!!!!!
		pJob->Dispatch();
	}
	else
		SDPA_LOG_ERROR("Gwes not initialized!");
}

/*
 * Implement here in a first phase a simple round-robin schedule
 */
void SchedulerImpl::schedule(const Job::ptr_t &job) {
	SDPA_LOG_DEBUG("Called schedule ...");


}

void SchedulerImpl::handleJob(Job::ptr_t& pJob)
{
	ostringstream os;
	os<<"Ask the scheduler to handle the job "<<pJob->id();
	SDPA_LOG_DEBUG(os.str());
	jobs_to_be_scheduled.push(pJob);
}

Worker::ptr_t SchedulerImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	try {
		return ptr_worker_man_->findWorker(worker_id);
	}
	catch(WorkerNotFoundException)
	{
		throw WorkerNotFoundException(worker_id);
	}
}

void SchedulerImpl::addWorker(const Worker::ptr_t pWorker)
{
	ptr_worker_man_->addWorker(pWorker);
}


void SchedulerImpl::start()
{
   assert(!m_thread);
   m_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&SchedulerImpl::run, this)));
   SDPA_LOG_DEBUG("Scheduler thread started ...");
   bStopRequested = false;
}

void SchedulerImpl::stop()
{
   assert(m_thread);
   bStopRequested = true;
   m_thread->interrupt();
   m_thread->join();
   SDPA_LOG_DEBUG("Scheduler thread stopped ...");
}

void SchedulerImpl::run()
{
	SDPA_LOG_DEBUG("Scheduler thread running ...");

	while(!bStopRequested)
	{
		try {
			Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait();

			if(pJob->is_local())
				schedule_local(pJob);
			else
				schedule(pJob);
		}
		catch( boost::thread_interrupted )
		{
			SDPA_LOG_DEBUG("Thread interrupted ...");
			bStopRequested = true;
		}
		catch( sdpa::daemon::QueueEmpty)
		{
			SDPA_LOG_DEBUG("Q empty exception");
			bStopRequested = true;
		}
	}
}
