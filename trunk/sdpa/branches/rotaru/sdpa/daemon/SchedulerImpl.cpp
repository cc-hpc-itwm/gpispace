/*
 * =====================================================================================
 *
 *       Filename:  SchedulerImpl.hpp
 *
 *    Description:  Implements a simple scheduler
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
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/id_generator.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SchedulerImpl::SchedulerImpl(sdpa::daemon::IComm* pCommHandler )
  : ptr_worker_man_(new WorkerManager())
  , ptr_comm_handler_(pCommHandler)
  , SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name():"tests::sdpa::SchedulerImpl")
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_last_request_time(0)
  , m_last_life_sign_time(0)
{
}

SchedulerImpl::~SchedulerImpl()
{
	SDPA_LOG_DEBUG("Called the destructor of  SchedulerImpl ...");
	try  {
		stop();
	}
	catch (...)
	{
		SDPA_LOG_DEBUG("Scheduler NRE running ...");
	}
}

/*
	Schedule a job locally, send the job to WE
*/
void SchedulerImpl::schedule_local(const sdpa::job_id_t &jobId)
{
	SDPA_LOG_DEBUG("Called schedule_local ...");

	id_type wf_id = jobId.str();

	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("Cannot schedule locally the job "<<jobId<<"! No communication handler specified.");
		stop();
		return;
	}

	try {

		const Job::ptr_t& pJob = ptr_comm_handler_->jobManager()->findJob(jobId);
		// put the job into the running state
		pJob->Dispatch();

		if( ptr_comm_handler_->workflowEngine() )
		{
			// Should set the workflow_id here, or send it together with the workflow description
			SDPA_LOG_DEBUG("Submit the workflow attached to the job "<<wf_id<<" to WE");
			ptr_comm_handler_->workflowEngine()->submit(wf_id, pJob->description());
		}
		else
		{
			SDPA_LOG_ERROR("Gwes not initialized or workflow not created!");
			//send a JobFailed event

			sdpa::job_result_t result;
			JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->name(), pJob->id(), result) );
			ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);

			if(!ptr_comm_handler_)
		    {
				stop();
				SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. "<<jobId.str());
		    }
		}
	}
	catch(JobNotFoundException& ex)
	{
		SDPA_LOG_DEBUG("Job not found! Could not schedule locally the job "<<ex.job_id().str());
	}
	catch (std::exception& ex)
	{
		SDPA_LOG_DEBUG("Exception occurred when trying to submit the workflow "<<wf_id<<" to WE: "<<ex.what());

		//send a JobFailed event
		sdpa::job_result_t result;
		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->name(), jobId, result) );
		ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
	}
}

/*
 * Implement here in a first phase a simple round-robin schedule
 */
void SchedulerImpl::schedule_remote(const sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("Called schedule_remote ...");

	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. "<<jobId.str());
		stop();
		return;
	}

	try {

		if( ptr_worker_man_ )
		{
			const Job::ptr_t& pJob = ptr_comm_handler_->jobManager()->findJob(jobId);

			SDPA_LOG_DEBUG("Get the next worker ...");
			Worker::ptr_t& pWorker = ptr_worker_man_->getNextWorker();

			SDPA_LOG_DEBUG("The job "<<pJob->id()<<" was assigned to the worker '"<<pWorker->name()<<"'!");

			pJob->worker() = pWorker->name();
			pWorker->dispatch(jobId);
		}
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

// obsolete, only for testing purposes!
void SchedulerImpl::start_job(const sdpa::job_id_t &jobId) {
	SDPA_LOG_DEBUG("Start the job "<<jobId.str());
}

void SchedulerImpl::schedule(sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("Handle job "<<jobId.str());
	jobs_to_be_scheduled.push(jobId);
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
   if(!ptr_comm_handler_)
   {
	   SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
	   return;
   }

   m_thread = boost::thread(boost::bind(&SchedulerImpl::run, this));
   SDPA_LOG_DEBUG("Scheduler thread started ...");
}

void SchedulerImpl::stop()
{
   m_thread.interrupt();

   bStopRequested = true;
   SDPA_LOG_DEBUG("Scheduler thread before join ...");
   m_thread.join();

   SDPA_LOG_DEBUG("Scheduler thread joined ...");
}

bool SchedulerImpl::post_request(bool force)
{
	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
		stop();
		return false;
	}

	bool bReqPosted = false;
	sdpa::util::time_type current_time = sdpa::util::now();
	sdpa::util::time_type difftime = current_time - m_last_request_time;

	if(force || (sdpa::daemon::ORCHESTRATOR != ptr_comm_handler_->name() &&  ptr_comm_handler_->is_registered()) )
	{
		if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval") )
		{
			// post a new request to the master
			// the slave posts a job request
			SDPA_LOG_DEBUG("Post a new request to "<<ptr_comm_handler_->master());
			RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
			ptr_comm_handler_->sendEventToMaster(pEvtReq);
			m_last_request_time = current_time;
			bReqPosted = true;
		}
	}

	return bReqPosted;
}

void SchedulerImpl::send_life_sign()
{
	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
		stop();
		return ;
	}

	 sdpa::util::time_type current_time = sdpa::util::now();
	 sdpa::util::time_type difftime = current_time - m_last_life_sign_time;

	 if( sdpa::daemon::ORCHESTRATOR != ptr_comm_handler_->name() &&  ptr_comm_handler_->is_registered() )
	 {
		 if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("life-sign interval") )
		 {
			 LifeSignEvent::Ptr pEvtLS( new LifeSignEvent( ptr_comm_handler_->name(), ptr_comm_handler_->master() ) );
			 ptr_comm_handler_->sendEventToMaster(pEvtLS);
			 m_last_life_sign_time = current_time;
		 }
	 }
}

void SchedulerImpl::check_post_request()
{
	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
		stop();
		return;
	}

	 if( sdpa::daemon::ORCHESTRATOR != ptr_comm_handler_->name() &&  ptr_comm_handler_->is_registered() )
	 {
		 //SDPA_LOG_DEBUG("Check if a new request is to be posted");
		 // post job request if number_of_jobs() < #registered workers +1
		 if( jobs_to_be_scheduled.size() <= numberOfWorkers() + 3 )
			 post_request();
		 else //send a LS
			 send_life_sign();
	 }
}

void SchedulerImpl::run()
{
	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
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
				// if it's an NRE just execute it!
				// Attention!: an NRE has no WorkerManager!!!!
				// or has an Worker Manager and the workers are threads
				schedule_remote(jobId);
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

void SchedulerImpl::print()
{
	jobs_to_be_scheduled.print();
	ptr_worker_man_->print();
}
