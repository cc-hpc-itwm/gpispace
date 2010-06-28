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

void SchedulerImpl::addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException)
{
	try {
		ptr_worker_man_->addWorker(workerId, rank);
		// only with a round-robin schedule
		// ptr_worker_man_->balanceWorkers();

		// with the "schedule job to the least loaded worker" strategy -> apply work stealing
	}
	catch( const WorkerAlreadyExistException& ex)
	{
		throw ex;
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

		const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);
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
 * Round-Robin schedule
 */
void SchedulerImpl::schedule_round_robin(const sdpa::job_id_t& jobId)
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
			const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

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


// test if the specified rank is valid
bool SchedulerImpl::schedule_to(const sdpa::job_id_t& jobId, unsigned int rank )
{
	SDPA_LOG_DEBUG("Schedule job "<<jobId.str()<<" to rank "<<rank);

	if( ptr_worker_man_->rank_map_.find(rank) == ptr_worker_man_->rank_map_.end() )
	{
		SDPA_LOG_WARN( "There is no worker with the rank= "<<rank<<"!" );
		return false;
	}

	try {

		const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

		Worker::worker_id_t worker_id = ptr_worker_man_->rank_map_.at(rank);

		Worker::ptr_t& pWorker = findWorker( worker_id);

		SDPA_LOG_DEBUG("The job "<<pJob->id()<<" was assigned to the worker '"<<pWorker->name()<<"'!");
		pJob->worker() = pWorker->name();
		pWorker->dispatch(jobId);

		return true;
	}
	catch(const WorkerNotFoundException& ex)
	{
		SDPA_LOG_WARN( "There is no worker with the rank= "<<rank<<"! Proceeding with the next rank ..." );
		return false;
	}
	catch(JobNotFoundException& ex)
	{
		SDPA_LOG_ERROR("Job not found! Could not schedule locally the job "<<ex.job_id().str());
		return false;
	}
}


/*
 * Scheduling with constraints
 */
bool SchedulerImpl::schedule_with_constraints(const sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("Called schedule_remote ...");

	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. "<<jobId.str());
		stop();
		return false;
	}

	if( ptr_worker_man_ )
	{
		if( ptr_worker_man_->numberOfWorkers()==0 )
		{
			ptr_comm_handler_->workerJobFailed( jobId, "No worker available!");
			return false;
		}

		const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

		// if no preferences are explicitly set for this job
        SDPA_LOG_DEBUG("Check if the job "<<jobId.str()<<" has preferences ... ");

		try
		{
			const we::preference_t& job_pref = ptr_comm_handler_->getJobPreferences(jobId);

			// the preferences not specified
			if( job_pref.empty() )
			{
				if(job_pref.is_mandatory())
				{
					ptr_comm_handler_->workerJobFailed( jobId, "The list of nodes needed is empty!");
					return false;
				}
				else
				{
					//Put the job into the common_queue of the WorkerManager
					ptr_worker_man_->dispatchJob(jobId);
					return true;
				}
			}
			else // the job has preferences
			{
				bool bAssigned = false;
				we::preference_t::exclude_set_type uset_excluded = job_pref.exclusion();

				const we::preference_t::rank_list_type& list_prefs=job_pref.ranks();
				for( we::preference_t::rank_list_type::const_iterator it = list_prefs.begin(); it != list_prefs.end() && !bAssigned; it++ )
					if(!(bAssigned=schedule_to(jobId, *it)))
						uset_excluded.insert(*it);

				// if the assignment to one of the preferred workers
				// fails and mandatory is set then -> declare the job failed
				if( !bAssigned && job_pref.is_mandatory() )
				{
					ptr_comm_handler_->workerJobFailed( jobId, "Couldn't match the mandatory preferences with an existing registered worker!");
					return false;
				}

				// if the assignement on one of preferred workers
				// fails and NOT mandatory is set then try to schedule it successively on one
				// of the remaining nodes that are not into the uset_excluded list and least loaded
				if( !bAssigned && job_pref.is_mandatory() ) // continue with the rest of the workers not uset_excluded
				{
					bAssigned = false;

					// declare the job failed!!!
					for( WorkerManager::rank_map_t::const_iterator iter = ptr_worker_man_->rank_map_.begin(); iter != ptr_worker_man_->rank_map_.end() && !bAssigned; iter++ )
					{
						unsigned int rank = iter->first;
						if( uset_excluded.find(rank) != uset_excluded.end() && !(bAssigned=schedule_to(jobId, rank)) )
								uset_excluded.insert(rank);
					}
				}

				return bAssigned;
			}
		}
		catch(const NoJobPreferences& )
		{
			//Put the job into the common_queue of the WorkerManager
			ptr_worker_man_->dispatchJob(jobId);
			return true;
		}
	}
	else
	{
		ptr_comm_handler_->workerJobFailed( jobId, "No worker available!");
		return false;
	}


	return false;
}

void SchedulerImpl::schedule_remote(const sdpa::job_id_t& jobId)
{
	schedule_with_constraints(jobId);

	// schedule_round_robin(jobId);
	// fairly re-distribute tasks, if necessary
	// ptr_worker_man_->balanceWorkers();
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
			const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

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

sdpa::job_id_t SchedulerImpl::getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	try {
		return ptr_worker_man_->getNextJob(worker_id, last_job_id);
	}
	catch(const NoJobScheduledException& ex)
	{
		throw ex;
	}
}

void SchedulerImpl::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException)
{

	SDPA_LOG_DEBUG("Acknowledge the job "<<job_id.str());

	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);

		//put the job into the Running state: do this in acknowledge!
		if( !ptrWorker->acknowledge(job_id) )
			throw JobNotFoundException(job_id);

	}
	catch(WorkerNotFoundException)
	{
		SDPA_LOG_DEBUG("Worker "<<worker_id<<" not found!");
	} catch(...) {
		SDPA_LOG_DEBUG("Unexpected exception occurred!");
	}
}
