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

#include <cassert>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SchedulerImpl::SchedulerImpl(sdpa::daemon::IComm* pCommHandler )
  : ptr_worker_man_(new WorkerManager())
  , ptr_comm_handler_(pCommHandler)
  , SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name():"tests::sdpa::SchedulerImpl")
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_last_request_time(0)
{
}

SchedulerImpl::~SchedulerImpl()
{
	SDPA_LOG_INFO( "Called the destructor of  SchedulerImpl ...");
	try  {
		stop();
	}
	catch (std::exception const & ex)
	{
		SDPA_LOG_ERROR("exception during SchedulerImpl::stop(): " << ex.what());
	}
	catch (...)
	{
		SDPA_LOG_ERROR("unexpected exception during SchedulerImpl::stop()");
	}
}

void SchedulerImpl::addWorker( const Worker::worker_id_t& workerId, unsigned int rank, const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
	try {
		ptr_worker_man_->addWorker(workerId, rank, agent_uuid);
		// only with a round-robin schedule
		// ptr_worker_man_->balanceWorkers();

		// with the "schedule job to the least loaded worker" strategy -> apply work stealing
	}
	catch( const WorkerAlreadyExistException& ex)
	{
		throw ex;
	}
}

void SchedulerImpl::re_schedule( Worker::JobQueue* pQueue )
{
  assert (pQueue);

	while( !pQueue->empty() )
	{
		sdpa::job_id_t jobId = pQueue->pop_and_wait();
		SDPA_LOG_INFO("Re-scheduling the job "<<jobId.str()<<" ... ");
		schedule_with_constraints(jobId);
	}
}

void SchedulerImpl::declare_jobs_failed(const Worker::worker_id_t& worker_id, Worker::JobQueue* pQueue )
{
  assert (pQueue);

  while( !pQueue->empty() )
  {
	  sdpa::job_id_t jobId = pQueue->pop_and_wait();
	  SDPA_LOG_INFO( "Declare the job "<<jobId.str()<<" failed!" );

	  if( ptr_comm_handler_ )
		  ptr_comm_handler_->workerJobFailed(worker_id, jobId, "Worker timeout detected!" );
	  else {
		  SDPA_LOG_ERROR("Invalid communication handler!");
	  }
  }
}

void SchedulerImpl::re_schedule( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
	// first re-schedule the work:
	// inspect all queues and re-schedule each job
	try {
		const Worker::ptr_t& pWorker = findWorker(worker_id);

		pWorker->set_timedout();

		// The jobs submitted by the WE should have set a property
		// which indicates whether the daemon can safely re-schedule these activities or not (reason: ex global mem. alloc)

		// for each job in the queue, either re-schedule it, if is allowed
		// re_schedule( &pWorker->acknowledged() );
		// re_schedule( &pWorker->submitted() );
		// or declare it failed

		// declare the submitted jobs failed
		declare_jobs_failed( worker_id, &pWorker->submitted() );

		// declare the acknowledged jobs failed
		declare_jobs_failed( worker_id, &pWorker->acknowledged() );

		// re-schedule the pending jobs
		re_schedule( &pWorker->pending() );
	}
	catch (const WorkerNotFoundException& ex)
	{
		SDPA_LOG_ERROR("Cannot delete the worker "<<worker_id<<". Worker not found!");
		throw ex;
	}
}

void SchedulerImpl::delWorker( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
	// first re-schedule the work:
	// inspect all queues and re-schedule each job
	try {
		re_schedule(worker_id);

		{
		  const Worker::ptr_t& pWorker = findWorker(worker_id);
		  LOG_IF( FATAL
		  	  ,  pWorker->pending().size()
		  	  || pWorker->submitted().size()
		  	  || pWorker->acknowledged().size()
		  	  , "tried to remove worker " << worker_id << " while there are still jobs scheduled!"
		  	  );
        }
		// delete the worker from the worker map
		ptr_worker_man_->delWorker(worker_id);
	}
	catch (const WorkerNotFoundException& ex)
	{
		SDPA_LOG_ERROR("Cannot delete the worker "<<worker_id<<". Worker not found!");
		throw ex;
	}
}

void SchedulerImpl::detectTimedoutWorkers( sdpa::util::time_type const & timeout )
{
	ptr_worker_man_->detectTimedoutWorkers(timeout);
}

// move this to the monitoring service
void SchedulerImpl::deleteNonResponsiveWorkers( sdpa::util::time_type const & timeout )
{
  // mark timedout workers and reschedule  their work...
  std::vector<Worker::worker_id_t> nonResponsive;
  ptr_worker_man_->detectTimedoutWorkers (timeout, &nonResponsive);
  std::for_each ( nonResponsive.begin()
                , nonResponsive.end()
                , boost::bind ( &SchedulerImpl::delWorker
                              , this
                              , _1
                              )
                );
}

/*
	Schedule a job locally, send the job to WE
*/
void SchedulerImpl::schedule_local(const sdpa::job_id_t &jobId)
{
    SDPA_LOG_DEBUG("Called schedule_local ...");

	id_type wf_id = jobId.str();

	if( !ptr_comm_handler_ )
	{
		SDPA_LOG_ERROR("Cannot schedule locally the job "<<jobId<<"! No communication handler specified.");
		stop();
		return;
	}

	try {
		const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

		// Should set the workflow_id here, or send it together with the workflow description
		SDPA_LOG_DEBUG("Submit the workflow attached to the job "<<wf_id<<" to WE. Workflow description follows: ");
		SDPA_LOG_DEBUG(pJob->description());

		pJob->Dispatch();

		if(pJob->description().empty() )
		{
			SDPA_LOG_ERROR("Empty Workflow!!!!");
			// declare job as failed
		}

		ptr_comm_handler_->submitWorkflow(wf_id, pJob->description());
	}
	catch(const NoWorkflowEngine& ex)
	{
		SDPA_LOG_ERROR("No workflow engine!!!");
		sdpa::job_result_t result(ex.what());

		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( sdpa::daemon::WE, ptr_comm_handler_->name(), jobId, result) );
		ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
	}
	catch(const JobNotFoundException& ex)
	{
		SDPA_LOG_ERROR("Job not found! Could not schedule locally the job "<<ex.job_id().str());
		sdpa::job_result_t result(ex.what());

		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( sdpa::daemon::WE, ptr_comm_handler_->name(), jobId, result) );
		ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
	}
	catch (std::exception const & ex)
	{
		SDPA_LOG_ERROR("Exception occurred when trying to submit the workflow "<<wf_id<<" to WE: "<<ex.what());

		//send a JobFailed event
		sdpa::job_result_t result(ex.what());

		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( sdpa::daemon::WE, ptr_comm_handler_->name(), jobId, result) );
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
		SDPA_LOG_ERROR("Invalid communication handler. "<<jobId.str());
		stop();
		return;
	}

	try {

		if( ptr_worker_man_ )
		{
			SDPA_LOG_DEBUG("Get the next worker ...");
			const Worker::ptr_t& pWorker = ptr_worker_man_->getNextWorker();

			SDPA_LOG_DEBUG("The job "<<jobId<<" was assigned to the worker '"<<pWorker->name()<<"'!");

			pWorker->dispatch(jobId);
		}
	}
	catch(const NoWorkerFoundException&)
	{
		// put the job back into the queue
		ptr_comm_handler_->workerJobFailed("", jobId, "No worker available!");
		SDPA_LOG_DEBUG("Cannot schedule the job. No worker available! Put the job back into the queue.");
	}
}

/*
 * return true only if scheduling the job jobid on the worker with the rank 'rank' succeeded
 */
// test if the specified rank is valid
bool SchedulerImpl::schedule_to(const sdpa::job_id_t& jobId, unsigned int rank, const preference_t& job_pref  )
{
	// attention! rank might not be of one of the preferred nodes when the preferences are not mandatory!
	SDPA_LOG_DEBUG("Schedule job "<<jobId.str()<<" to rank "<<rank);

	if( ptr_worker_man_->rank_map().find(rank) == ptr_worker_man_->rank_map().end() )
	{
		SDPA_LOG_WARN( "There is no worker with the rank= "<<rank<<"!" );
		return false;
	}

	// if the worker is marked for deletion don't schedule any job on it
	// should have a monitoring thread that detects the timedout nodes
	// add a boolean variable to the worker bTimedout or not
	try
	{
		const Worker::worker_id_t worker_id = ptr_worker_man_->worker(rank);
		const Worker::ptr_t& pWorker = findWorker( worker_id);

		if( pWorker->timedout() )
		{
			SDPA_LOG_WARN("Couldn't schedule the job "<<jobId.str()<<" on the worker "<<worker_id<<". Timeout detected!");
			return false;
		}

		SDPA_LOG_DEBUG("The job "<<jobId<<" was assigned to the worker '"<<pWorker->name()<<"'!");

		pWorker->dispatch(jobId);
		ptr_worker_man_->make_owner(jobId, worker_id);

		const preference_t::rank_list_type& list_ranks = job_pref.ranks();

		int k=0;
		for( preference_t::rank_list_type::const_iterator it = list_ranks.begin(); it != list_ranks.end(); it++ )
			if( ptr_worker_man_->worker(*it) != worker_id )
			{
				Worker::pref_deg_t pref_deg = k++;

				unsigned int rank = *it;
				const Worker::ptr_t& pOtherWorker = ptr_worker_man_->findWorker( ptr_worker_man_->worker(rank) );
				pOtherWorker->add_to_affinity_list(pref_deg, jobId);
			}

		return true;
	}
	catch( const NoWorkerFoundException& ex1)
	{
		SDPA_LOG_WARN("Couldn't schedule the job "<<jobId.str()<<". No worker with the rank= "<<rank<<" found!" );
		return false;
	}
	catch(const WorkerNotFoundException& ex2)
	{
		SDPA_LOG_WARN( "Couldn't schedule the job "<<jobId.str()<<". There is no worker with the rank= "<<rank );
		return false;
	}
}

void SchedulerImpl::schedule_anywhere( const sdpa::job_id_t& jobId )
{
	ptr_worker_man_->dispatchJob(jobId);
}

/*
 * Scheduling with constraints
 */
bool SchedulerImpl::schedule_with_constraints(const sdpa::job_id_t& jobId,  bool bDelNonRespWorkers )
{
	DLOG(TRACE, "Called schedule_with_contraints ...");

	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("Invalid communication handler. "<<jobId.str());
		stop();
		return false;
	}

	if( ptr_worker_man_ )
	{
		// if no preferences are explicitly set for this job
		LOG(TRACE, "Check if the job "<<jobId.str()<<" has preferences ... ");

		try
		{
			const preference_t& job_pref = ptr_comm_handler_->getJobPreferences(jobId);

			// the preferences not specified
			if( job_pref.empty() )
			{
				if(job_pref.is_mandatory())
				{
					LOG(WARN, "a job requested an empty list of mandatory nodes: " << jobId);
					ptr_comm_handler_->workerJobFailed("", jobId, "The list of nodes needed is empty!");
					return false;
				}
				else
				{
					// schedule to the first worker that requests a job
					schedule_anywhere(jobId);
					return true;
				}
			}
			else // the job has preferences
			{
				preference_t::exclude_set_type uset_excluded = job_pref.exclusion();

				const preference_t::rank_list_type& list_prefs=job_pref.ranks();
				for( preference_t::rank_list_type::const_iterator it = list_prefs.begin(); it != list_prefs.end(); it++ )
				{
					// use try-catch for the case when the no worker with that rank exists
					if( schedule_to(jobId, *it, job_pref) )
						return true;
					else
						uset_excluded.insert(*it);
                }

				// if the assignment to one of the preferred workers
				// fails and mandatory is set then -> declare the job failed
				if( job_pref.is_mandatory() )
				{
					LOG(WARN, "Couldn't match the mandatory preferences with a registered worker: job-id := " << jobId << " pref := " << job_pref);
					ptr_comm_handler_->workerJobFailed("", jobId, "Couldn't match the mandatory preferences with a registered worker!");
					return false;
				}
				else  // continue with the rest of the workers not in uset_excluded
				{
					std::vector<unsigned int> registered_ranks;
					ptr_worker_man_->getListOfRegisteredRanks(registered_ranks);

					for( std::vector<unsigned int>::const_iterator iter (registered_ranks.begin())
                                           ; iter != registered_ranks.end(); iter++ )
						// return immediately if rank not excluded and scheduling to rank succeeded
						if( uset_excluded.find(*iter) == uset_excluded.end() && schedule_to(jobId, *iter, job_pref) )
							return true;
				}

                // TODO: we had preferences but we could not fulfill them
				ptr_comm_handler_->workerJobFailed("", jobId, "The job had preferences which could not be fulfilled!");
				return false;
			}
		}
		catch(const NoJobPreferences& )
		{
			// schedule to the first worker that requests a job
			schedule_anywhere(jobId);
			return true;
		}
	}
	else
	{
		LOG(WARN, "could not schedule job: no worker available: " << jobId);
		ptr_comm_handler_->workerJobFailed("", jobId, "No worker available!");
		return false;
	}


	return false;
}

void SchedulerImpl::schedule_remote(const sdpa::job_id_t& jobId)
{
	// check if there are any responsive workers left
	// delete non_responsive workers

	// schedule_round_robin(jobId);
	// fairly re-distribute tasks, if necessary
	// ptr_worker_man_->balanceWorkers();
	// fix this later -> use a monitoring thread
	//if(bDelNonRespWorkers)
	if(ptr_comm_handler_->cfg()->is_set("worker_timeout"))
	{
		//SDPA_LOG_WARN("Delete timed-out/non-responsive workers!");
		deleteNonResponsiveWorkers (ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("worker_timeout"));
	}

	if( !numberOfWorkers() )
	{
		SDPA_LOG_WARN("No worker found. The job " << jobId<<" wasn't assigned to any worker. Try later!");
		throw NoWorkerFoundException();
	}
	else
		schedule_with_constraints(jobId);
}

// obsolete, only for testing purposes!
void SchedulerImpl::start_job(const sdpa::job_id_t &jobId) {
	SDPA_LOG_DEBUG("Start the job "<<jobId.str());
}

void SchedulerImpl::schedule(const sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("Schedule the job " << jobId.str());
	jobs_to_be_scheduled.push(jobId);
}

const Worker::ptr_t& SchedulerImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	try {
		return ptr_worker_man_->findWorker(worker_id);
	}
	catch(WorkerNotFoundException)
	{
		throw WorkerNotFoundException(worker_id);
	}
}

const Worker::worker_id_t& SchedulerImpl::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
	try {
		return ptr_worker_man_->findWorker(job_id);
	}
	catch(const NoWorkerFoundException& ex)
	{
		throw ex;
	}
}

void SchedulerImpl::start(IComm* p)
{
	if(p)
		ptr_comm_handler_ = p;

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
   bStopRequested = true;
   m_thread.interrupt();
   DLOG(TRACE, "Scheduler thread before join ...");
   m_thread.join();

   DLOG(TRACE, "Scheduler thread joined ...");

   LOG_IF( WARN
         , jobs_to_be_scheduled.size()
         , "there are still jobs to be scheduled: " << jobs_to_be_scheduled.size()
         );

   ptr_comm_handler_ = NULL;
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

	if(force || ( !ptr_comm_handler_->is_orchestrator()  &&  ptr_comm_handler_->is_registered() ) )
	{
		if( difftime > ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("polling interval") )
		{
			// post a new request to the master
			// the slave posts a job request
			ptr_comm_handler_->requestJob();

			m_last_request_time = current_time;
			bReqPosted = true;
		}
	}

	return bReqPosted;
}

void SchedulerImpl::check_post_request()
{
	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
		stop();
		return;
	}

	if( !ptr_comm_handler_->is_orchestrator())
	{
		// TODO: remove requests
        if (ptr_comm_handler_->is_registered() && jobs_to_be_scheduled.size() <= numberOfWorkers() + 3)
        {
        	post_request();
        }
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

			if( pJob->is_local() )
				schedule_local(jobId);
			else
			{
				// if it's an NRE just execute it!
				// Attention!: an NRE has no WorkerManager!!!!
				// or has an Worker Manager and the workers are threads

				try
				{
					schedule_remote(jobId);
				}
				catch( const NoWorkerFoundException& ex)
				{
					SDPA_LOG_DEBUG("No valid worker found! Put the job "<<jobId.str()<<" into the common queue");
					// do so as when no preferences were set, just ignore them right now
					//schedule_anywhere(jobId);

					ptr_worker_man_->dispatchJob(jobId);
				}
			}
		}
		catch(const JobNotFoundException& ex)
		{
			SDPA_LOG_DEBUG("Job not found! Could not schedule the job "<<ex.job_id().str());
            throw;
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
            throw;
		}
	}
}

void SchedulerImpl::print()
{
	if(!jobs_to_be_scheduled.empty())
	{
		SDPA_LOG_DEBUG("The content of agent's scheduler queue is:");
		jobs_to_be_scheduled.print();
	}
	else
		SDPA_LOG_DEBUG("No job to be scheduled left!");

	SDPA_LOG_DEBUG("The content of agent's WorkerManager is:");
	ptr_worker_man_->print();
}

const sdpa::job_id_t SchedulerImpl::getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException)
{
	sdpa::job_id_t job_id = sdpa::job_id_t::invalid_job_id();

	try {
		job_id = ptr_worker_man_->getNextJob(worker_id, last_job_id);
	}
	catch(const NoJobScheduledException& ex1)
	{
		//SDPA_LOG_ERROR("Exception: no jobs scheduled!");
		throw ex1;
	}
	catch(WorkerNotFoundException& ex2)
	{
		//SDPA_LOG_ERROR("Exception occurred: worker not found!");
		throw ex2;
	}

	return job_id;
}

void SchedulerImpl::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException)
{
	DLOG(TRACE, "Acknowledge the job "<<job_id.str());
	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);

		//put the job into the Running state: do this in acknowledge!
		if( !ptrWorker->acknowledge(job_id) )
			throw JobNotFoundException(job_id);
	}
	catch(JobNotFoundException const& ex1)
	{
		SDPA_LOG_ERROR("Could not find the job "<<job_id.str()<<"!");
		throw ex1;
	}
	catch(WorkerNotFoundException const &ex2 )
	{
		SDPA_LOG_ERROR("The worker "<<worker_id<<" could not be found!");
		throw ex2;
	}
}

void SchedulerImpl::deleteWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException)
{
	try {
		ptr_worker_man_->deleteWorkerJob(worker_id, job_id);
	}
	catch(JobNotDeletedException const& ex1)
	{
		SDPA_LOG_ERROR("Could not delete the job "<<job_id.str()<<"!");
		throw ex1;
	}
	catch(WorkerNotFoundException const &ex2 )
	{
		SDPA_LOG_ERROR("The worker "<<worker_id<<" could not be found!");
		throw ex2;
	}
	catch(const std::exception& ex3 )
	{
		SDPA_LOG_ERROR("Unexpected exception occurred when trying to delete the job "<<job_id.str()<<" from the worker "<<worker_id<<": "<< ex3.what() );
		throw ex3;
	}
}

bool SchedulerImpl::has_job(const sdpa::job_id_t& job_id)
{
	if( jobs_to_be_scheduled.find(job_id) != jobs_to_be_scheduled.end() )
	{
		return true;
	}

	return ptr_worker_man_->has_job(job_id);
}


void SchedulerImpl::notifyWorkers(const sdpa::events::ErrorEvent::error_code_t& errcode)
{
	std::list<std::string> workerList;
	ptr_worker_man_->getWorkerList(workerList);

	if( ptr_comm_handler_ )
	{
		if(workerList.empty())
		{
			SDPA_LOG_INFO("The worker list is empty. No worker to be notified exist!");
			return;
		}

		for( std::list<std::string>::iterator iter = workerList.begin(); iter != workerList.end(); iter++ )
		{
			SDPA_LOG_INFO("Send notification to the worker "<<*iter<<" "<< errcode<<" ...");
			ErrorEvent::Ptr pErrEvt(new ErrorEvent( ptr_comm_handler_->name(), *iter, errcode,  "worker notification") );
			//ErrorEvent::SDPA_EWORKERNOTREG
			ptr_comm_handler_->sendEventToMaster(pErrEvt);
		}
	}
	else
		SDPA_LOG_ERROR("No valid communication handler!");
}
