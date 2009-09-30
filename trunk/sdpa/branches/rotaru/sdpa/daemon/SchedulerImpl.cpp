#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>

using namespace sdpa::daemon;

SchedulerImpl::SchedulerImpl(sdpa::wf::Sdpa2Gwes*  pSdpa2Gwes):
	ptr_Sdpa2Gwes_(pSdpa2Gwes),
	ptr_worker_man_(new WorkerManager()),
	SDPA_INIT_LOGGER("sdpa::daemon::SchedulerImpl")
{
}

SchedulerImpl::~SchedulerImpl()
{
	SDPA_LOG_DEBUG("Called the destructor of  SchedulerImpl ...");
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

	sdpa::wf::workflow_id_t wf_id = pJob->id().str();
	sdpa::wf::workflow_t workflow(wf_id, pJob->description());

	// Should set the workflow_id here, or send it together with the workflow description
	ostringstream os;
	os<<"Submit the workflow attached to the job "<<wf_id<<" to GWES";
	SDPA_LOG_DEBUG(os.str());

	if(ptr_Sdpa2Gwes_)
	{
		ptr_Sdpa2Gwes_->submitWorkflow(workflow);

		//Put the job into Running state
		//send back to the user a SubmitJobAckEvent
		sdpa::events::SubmitJobAckEvent::Ptr pSubmAckEvt(new sdpa::events::SubmitJobAckEvent("", "", pJob->id()));
		pJob->Dispatch(pSubmAckEvt.get()); // no event need to be sent
	}
	else
		SDPA_LOG_ERROR("Gwes not initialized!");
}

/*
 * Implement here in a first phase a simple round-robin schedule
 */
void SchedulerImpl::schedule(const Job::ptr_t &pJob) {
	SDPA_LOG_DEBUG("Called schedule ...");

	try {
		SDPA_LOG_DEBUG("Get the next worker ...");
		Worker::ptr_t& pWorker = ptr_worker_man_->getNextWorker();

		SDPA_LOG_DEBUG("The next worker dispatches the job ...");
		pWorker->dispatch(pJob);
	}
	catch(NoWorkerFoundException&)
	{
		// put the job back into the queue
		jobs_to_be_scheduled.push(pJob);
		SDPA_LOG_DEBUG("Cannot schedule the job. No worker available!");
	}
}

void SchedulerImpl::handleJob(Job::ptr_t& pJob)
{
	ostringstream os;
	os<<"Handle job "<<pJob->id();
	SDPA_LOG_DEBUG(os.str());

	jobs_to_be_scheduled.push(pJob);
}

Worker::ptr_t &SchedulerImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	try {
		return ptr_worker_man_->findWorker(worker_id);
	}
	catch(WorkerNotFoundException)
	{
		throw WorkerNotFoundException(worker_id);
	}
}

void SchedulerImpl::addWorker(const Worker::ptr_t &pWorker)
{
	ptr_worker_man_->addWorker(pWorker);
}


void SchedulerImpl::start()
{
   bStopRequested = false;
   m_thread = boost::thread(boost::bind(&SchedulerImpl::run, this));
   SDPA_LOG_DEBUG("Scheduler thread started ...");
}

void SchedulerImpl::stop()
{
   m_thread.interrupt();
   //jobs_to_be_scheduled.stop();

   bStopRequested = true;
   SDPA_LOG_DEBUG("Scheduler thread before join ...");
   m_thread.join();

   ptr_Sdpa2Gwes_ = NULL;
   SDPA_LOG_DEBUG("Scheduler thread joined ...");
}

void SchedulerImpl::run()
{
	SDPA_LOG_DEBUG("Scheduler thread running ...");

	while(!bStopRequested)
	{
		try {
			Job::ptr_t pJob = jobs_to_be_scheduled.pop_and_wait();

			if( pJob.use_count() )
			{
				if(pJob->is_local())
					schedule_local(pJob);
				else //schedule(pJob);
				{
					SDPA_LOG_DEBUG("Get the next worker ...");
					Worker::ptr_t pWorker = ptr_worker_man_->getNextWorker();

					SDPA_LOG_DEBUG("The next worker dispatches the job ...");
					pWorker->dispatch(pJob);
				}
			}
			else
			{
				bStopRequested = true;
				SDPA_LOG_DEBUG("Thread stops now!");
			}
		}
		catch( const boost::thread_interrupted & )
		{
			SDPA_LOG_DEBUG("Thread interrupted ...");
			bStopRequested = true;
		}
		catch( const sdpa::daemon::QueueEmpty &)
		{
			SDPA_LOG_DEBUG("Queue empty exception");
			bStopRequested = true;
		}
	}
}
