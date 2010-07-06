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


void SchedulerImpl::re_schedule( Worker::JobQueue* pQueue )
{
	while( !pQueue->empty() )
	{
		sdpa::job_id_t jobId = pQueue->pop_and_wait();
		SDPA_LOG_INFO("Re-scheduling the job "<<jobId.str()<<" ... ");
		schedule_with_constraints(jobId);
	}
}

void SchedulerImpl::declare_jobs_failed( Worker::JobQueue* pQueue )
{
	while( !pQueue->empty() )
	{
		sdpa::job_id_t jobId = pQueue->pop_and_wait();
		SDPA_LOG_INFO("Declare the job "<<jobId.str()<<" failed!");

		if( ptr_comm_handler_ )
			ptr_comm_handler_->workerJobFailed( jobId, "Worker timeout detected!");
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

		// TODO we have to  think about the implications of rescheduling
		// already submitted jobs,  in the general case, it  is not safe
		// to reschedule them!  For  pending jobs we can reschedule them
		// without thinking twice, for all others we simply cannot!

		// The jobs submitted by the WE should have a property set
		// indicating if the daemon can safely re-schedule these activities or not (reason: ex global mem. alloc)
		// re_schedule( &pWorker->acknowledged() );
		// re_schedule( &pWorker->submitted() );

		// declare the submitted jobs failed
		declare_jobs_failed( &pWorker->submitted() );

		// declare the acknowledged jobs failed
		declare_jobs_failed( &pWorker->acknowledged() );

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

	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("Cannot schedule locally the job "<<jobId<<"! No communication handler specified.");
		stop();
		return;
	}

	try {
		const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

		// Should set the workflow_id here, or send it together with the workflow description
		SDPA_LOG_DEBUG("Submit the workflow attached to the job "<<wf_id<<" to WE");
		//ptr_comm_handler_->workflowEngine()->submit(wf_id, pJob->description());
		pJob->Dispatch();
		ptr_comm_handler_->submitWorkflow(wf_id, pJob->description());
	}
	catch(const NoWorkflowEngine& ex)
	{
		SDPA_LOG_ERROR("No workflow engine!!!");
		sdpa::job_result_t result;
		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->name(), jobId, result) );
		ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
	}
	catch(const JobNotFoundException& ex)
	{
		SDPA_LOG_ERROR("Job not found! Could not schedule locally the job "<<ex.job_id().str());
		sdpa::job_result_t result;
		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent( ptr_comm_handler_->name(), ptr_comm_handler_->name(), jobId, result) );
		ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
	}
	catch (std::exception const & ex)
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
		ptr_comm_handler_->workerJobFailed( jobId, "No worker available!");
		SDPA_LOG_DEBUG("Cannot schedule the job. No worker available! Put the job back into the queue.");
	}
}

/*
 * return true only if scheduling the job jobid on the worker with the rank 'rank' succeeded
 */
// test if the specified rank is valid
bool SchedulerImpl::schedule_to(const sdpa::job_id_t& jobId, unsigned int rank, const we::preference_t& job_pref  )
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
		const Worker::worker_id_t worker_id = ptr_worker_man_->rank_map().at(rank);
		const Worker::ptr_t& pWorker = findWorker( worker_id);

		if( pWorker->timedout() )
		{
			SDPA_LOG_WARN("Couldn't schedule the job "<<jobId.str()<<" on the worker "<<worker_id<<". Timeout detected!");
			return false;
		}

		SDPA_LOG_DEBUG("The job "<<jobId<<" was assigned to the worker '"<<pWorker->name()<<"'!");

		pWorker->dispatch(jobId);
		ptr_worker_man_->owner_map().insert(WorkerManager::owner_map_t::value_type(jobId,worker_id));

		// maintain a multi-index container with info about jobs preferring workers
		const we::preference_t::rank_list_type& list_ranks = job_pref.ranks();
		int k=0;
		for( we::preference_t::rank_list_type::const_iterator it = list_ranks.begin(); it != list_ranks.end(); it++ )
			if( ptr_worker_man_->worker(*it) != worker_id )
			{
				Worker::pref_deg_t pref_deg = k++;
				unsigned int rank = *it;

				const Worker::worker_id_t& other_worker_id = ptr_worker_man_->worker(rank);
				const Worker::ptr_t& pOtherWorker = ptr_worker_man_->findWorker(other_worker_id);

				pOtherWorker->mi_affinity_list.insert( Worker::scheduling_preference_t( pref_deg, jobId ) );
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


/*
 * Scheduling with constraints
 */
bool SchedulerImpl::schedule_with_constraints(const sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("Called schedule_with_contraints ...");

	if(!ptr_comm_handler_)
	{
		SDPA_LOG_ERROR("Invalid communication handler. "<<jobId.str());
		stop();
		return false;
	}

    // TODO  this call  is just  for  now here,  there should  be an  active
    // component checking dropped connections.

	// fix this later
	//deleteNonResponsiveWorkers (ptr_comm_handler_->cfg()->get<sdpa::util::time_type>("node_timeout"));

	if( ptr_worker_man_ )
	{
		if( ptr_worker_man_->numberOfWorkers()==0 )
		{
			LOG(WARN, "No worker registered, marking job as failed: " << jobId);

			ptr_comm_handler_->workerJobFailed( jobId, "No worker available!");
			return false;
		}

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
					LOG(WARN, "a job requested an empty list of mandatory nodes: " << jobId);
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
				we::preference_t::exclude_set_type uset_excluded = job_pref.exclusion();

				const we::preference_t::rank_list_type& list_prefs=job_pref.ranks();
				for( we::preference_t::rank_list_type::const_iterator it = list_prefs.begin(); it != list_prefs.end(); it++ )
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
					ptr_comm_handler_->workerJobFailed( jobId, "Couldn't match the mandatory preferences with a registered worker!");
					return false;
				}
				else  // continue with the rest of the workers not uset_excluded
				{
					std::vector<unsigned int> registered_ranks;
					ptr_worker_man_->getListOfRegisteredRanks(registered_ranks);

					for( std::vector<unsigned int>::const_iterator iter = registered_ranks.begin(); iter != registered_ranks.end(); iter++ )
					{
						if( uset_excluded.find(*iter) == uset_excluded.end() ) // the rank *iter is not excluded
						{
							if( schedule_to(jobId, *iter, job_pref) )
								return true;
						}
					}
				}

				return false;
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
          LOG(WARN, "could not schedule job: no worker available: " << jobId);
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
			SDPA_LOG_DEBUG("Job not found! Could not schedule the job "<<ex.job_id().str());
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

const sdpa::job_id_t SchedulerImpl::getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException)
{
  return ptr_worker_man_->getNextJob(worker_id, last_job_id);
}

void SchedulerImpl::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException)
{
  SDPA_LOG_DEBUG("Acknowledge the job "<<job_id.str());
  Worker::ptr_t ptrWorker = findWorker(worker_id);

  //put the job into the Running state: do this in acknowledge!
  if( !ptrWorker->acknowledge(job_id) )
    throw JobNotFoundException(job_id);
}

void SchedulerImpl::deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException)
{
  ptr_worker_man_->deleteWorkerJob(worker_id, job_id);
}

