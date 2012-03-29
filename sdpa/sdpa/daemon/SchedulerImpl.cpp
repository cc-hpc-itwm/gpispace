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
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <sdpa/events/id_generator.hpp>
#include <boost/tokenizer.hpp>

#include <cassert>
#include <sdpa/capability.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;


SchedulerImpl::SchedulerImpl(sdpa::daemon::IComm* pCommHandler, bool bUseRequestModel )
  : ptr_worker_man_(new WorkerManager())
  , ptr_comm_handler_(pCommHandler)
  , SDPA_INIT_LOGGER((pCommHandler?pCommHandler->name().c_str():"Scheduler"))
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_bUseRequestModel(bUseRequestModel)
{
}

SchedulerImpl::~SchedulerImpl()
{
  SDPA_LOG_INFO( "Called the destructor of  SchedulerImpl ...");
  try  {
		if( jobs_to_be_scheduled.size() )
		{
		  SDPA_LOG_WARN("The scheduler has still "<<jobs_to_be_scheduled.size()<<" jobs into his queue!");
		}

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

void SchedulerImpl::addWorker( 	const Worker::worker_id_t& workerId,
								const unsigned int& capacity,
								const capabilities_set_t& cpbset,
								const unsigned int& agent_rank,
								const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
  try {
      ptr_worker_man_->addWorker(workerId, capacity, cpbset, agent_rank, agent_uuid);
      cond_workers_registered.notify_all();
      // only with a round-robin schedule
      // ptr_worker_man_->balanceWorkers();

      // with the "schedule job to the least loaded worker" strategy -> apply work stealing
  }
  catch( const WorkerAlreadyExistException& ex)
  {
      throw ex;
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
      {
          ptr_comm_handler_->workerJobFailed(worker_id, jobId, "Worker timeout detected!" );
      }
      else
      {
          SDPA_LOG_ERROR("Invalid communication handler!");
      }
  }
}

void SchedulerImpl::reschedule( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
{
	ostringstream os;
	try {
		// delete it from the worker's queues
		Worker::ptr_t pWorker = findWorker(worker_id);
		pWorker->delete_job(job_id);

		Job::ptr_t pJob = ptr_comm_handler_->jobManager()->findJob(job_id);
		pJob->Reschedule(); // put the job back into the pending state

		schedule(job_id);
	}
	catch (const WorkerNotFoundException& ex)
	{
		SDPA_LOG_WARN("Cannot delete the worker "<<worker_id<<". Worker not found!");
	}
	catch(JobNotFoundException const &ex)
	{
		SDPA_LOG_WARN("Cannot re-schedule the job " << job_id << ". The job could not be found!");
	}
	catch(JobNotDeletedException const & ex)
	{
		SDPA_LOG_WARN("The job " << job_id << " could not be deleted: " << ex.what());
	}
	catch(const std::exception& ex) {
		SDPA_LOG_WARN( "Could not re-schedule the job " << job_id << ": unexpected error!"<<ex.what() );
	}
}

void SchedulerImpl::reassign( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
{
	ostringstream os;
	try {
		// delete it from the worker's queues
		Worker::ptr_t pWorker = findWorker(worker_id);
		pWorker->delete_job(job_id);

		Job::ptr_t pJob = ptr_comm_handler_->jobManager()->findJob(job_id);
		pJob->Reschedule(); // put the job back into the pending state

		pWorker->dispatch(job_id); // or schedule_to(job_id, worker_id);
	}
	catch (const WorkerNotFoundException& ex)
	{
		SDPA_LOG_WARN("Cannot delete the worker "<<worker_id<<". Worker not found!");
	}
	catch(JobNotFoundException const &ex)
	{
		SDPA_LOG_WARN("Cannot re-schedule the job " << job_id << ". The job could not be found!");
	}
	catch(JobNotDeletedException const & ex)
	{
		SDPA_LOG_WARN("The job " << job_id << " could not be deleted: " << ex.what());
	}
	catch(const std::exception& ex) {
		SDPA_LOG_WARN( "Could not re-schedule the job " << job_id << ": unexpected error!"<<ex.what() );
	}
}

void SchedulerImpl::reschedule( const Worker::worker_id_t & worker_id, Worker::JobQueue* pQueue )
{
	assert (pQueue);

	while( !pQueue->empty() )
	{
		sdpa::job_id_t jobId = pQueue->pop_and_wait();
		SDPA_LOG_INFO("Re-scheduling the job "<<jobId.str()<<" ... ");
		reschedule(worker_id, jobId);
	}
}

void SchedulerImpl::reschedule( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
  // first re-schedule the work:
  // inspect all queues and re-schedule each job
  try {
    const Worker::ptr_t& pWorker = findWorker(worker_id);

    // The jobs submitted by the WE should have set a property
    // which indicates whether the daemon can safely re-schedule these activities or not (reason: ex global mem. alloc)

    // for each job in the queue, either re-schedule it, if is allowed
    // re_schedule( &pWorker->acknowledged() );
    // re_schedule( &pWorker->submitted() );
    // or declare it failed
    pWorker->set_disconnected();

    // declare the submitted jobs failed
    //declare_jobs_failed( worker_id, &pWorker->submitted() );
    reschedule(worker_id, &pWorker->submitted() );

    // declare the acknowledged jobs failed
    //declare_jobs_failed( worker_id, &pWorker->acknowledged() );
    reschedule(worker_id, &pWorker->acknowledged() );

    // re-schedule the pending jobs
    reschedule(worker_id, &pWorker->pending() );

    // put the jobs back into the central queue and don't forget
    // to reset the status

  }
  catch (const WorkerNotFoundException& ex)
  {
    SDPA_LOG_ERROR("Could not find the worker "<<worker_id);
    throw ex;
  }
}

void SchedulerImpl::delWorker( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
  // first re-schedule the work:
  // inspect all queues and re-schedule each job
  try {

	  // mark the worker dirty -> don't take it in consideration for re-scheduling
	  const Worker::ptr_t& pWorker = findWorker(worker_id);

	  pWorker->set_disconnected(true);

	  reschedule(worker_id);
      if( !pWorker->pending().empty() )
      {
    	  SDPA_LOG_WARN( "The worker " << worker_id << " has still pending jobs!");
    	  pWorker->print();
      }

      if( !pWorker->submitted().empty() )
      {
    	  SDPA_LOG_WARN( "The worker " << worker_id << " has still submitted jobs and not acknowledged!");
    	  pWorker->print();
      }

      if( !pWorker->acknowledged().empty() )
      {
    	  SDPA_LOG_WARN( "The worker " << worker_id << " has still acknowledged jobs and not finished!");
    	  pWorker->print();
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

/*
	Schedule a job locally, send the job to WE
*/
void SchedulerImpl::schedule_local(const sdpa::job_id_t &jobId)
{
  SDPA_LOG_DEBUG("Schedule the job "<<jobId.str()<<" to the workflow engine!");

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
	  SDPA_LOG_DEBUG("Submit the workflow attached to the job "<<wf_id<<" to WE. ");
	  //SDPA_LOG_DEBUG("Workflow description follows: ");
	  //SDPA_LOG_DEBUG(pJob->description());

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

  try
  {
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
bool SchedulerImpl::schedule_to(const sdpa::job_id_t& jobId, const Worker::ptr_t& pWorker)
{
	sdpa::worker_id_t worker_id = pWorker->name();

	// attention! rank might not be of one of the preferred nodes when the preferences are not mandatory!
	SDPA_LOG_DEBUG("Schedule job "<<jobId.str()<<" to the worker "<<worker_id);

	// if the worker is marked for deletion don't schedule any job on it
	// should have a monitoring thread that detects the timed-out nodes
	// add a boolean variable to the worker bTimedout or not
	try
	{
		if( pWorker->timedout() )
		{
			SDPA_LOG_WARN("Couldn't schedule the job "<<jobId.str()<<" on the worker "<<worker_id<<". Timeout detected!");
			return false;
		}

		SDPA_LOG_DEBUG("The job "<<jobId<<" was assigned to the worker '"<<pWorker->name()<<"'!");

		pWorker->dispatch(jobId);
		return true;
	}
	catch( const NoWorkerFoundException& ex1)
	{
		SDPA_LOG_WARN("Couldn't schedule the job "<<jobId.str()<<". No worker named "<<worker_id<<" found!" );
		return false;
	}
	catch(const WorkerNotFoundException& ex2)
	{
		SDPA_LOG_WARN( "Couldn't schedule the job "<<jobId.str()<<". There is no worker "<<worker_id );
		return false;
	}
}

bool SchedulerImpl::schedule_to(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& workerId)
{
	const Worker::ptr_t& pWorker = findWorker( workerId);
	return schedule_to(jobId, pWorker);
}

void SchedulerImpl::delete_job (sdpa::job_id_t const & job)
{
	SDPA_LOG_DEBUG("removing job " << job << " from the scheduler's queue ....");
  	if (jobs_to_be_scheduled.erase(job))
	{
	  SDPA_LOG_DEBUG("removed job from the central queue...");
	}
	else
    	ptr_worker_man_->delete_job(job);
}

void SchedulerImpl::schedule_anywhere( const sdpa::job_id_t& jobId )
{
	ptr_worker_man_->dispatchJob(jobId);
}


/*
 * Scheduling with constraints
 */
bool SchedulerImpl::schedule_with_constraints( const sdpa::job_id_t& jobId )
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
      DLOG(TRACE, "Check if there are requirements specified for the job "<<jobId.str()<<"  ... ");

      try
      {
    	  const requirement_list_t job_req_list = ptr_comm_handler_->getJobRequirements(jobId);
          // no preferences specified
          if( job_req_list.empty() )
          {
        	  // schedule to the first worker that requests a job
              DLOG(TRACE, "The requirements list for the job "<<jobId<<" is empty. Schedule it anywhere!");
              schedule_anywhere(jobId);
              return true;
          }
          else // there are requirements specified for that job
          {
        	  try
        	  {
        		  // first round: get the list of all workers for which the mandatory requirements are matching the capabilities
        		  Worker::ptr_t ptrBestWorker = ptr_worker_man_->getBestMatchingWorker(job_req_list);

				  ostringstream ossReq;
				  BOOST_FOREACH(const requirement_t& req, job_req_list)
				  {
					ossReq<<req.value()<<",";
				  }

        		  LOG( TRACE
                             , "The best worker matching the requirements: "
                             << ossReq.str()
                             <<" for the job  " << jobId
                             << " is " << ptrBestWorker->name()
                             );

        		  // schedule the job to that one
        		  DMLOG(TRACE, "Schedule the job "<<jobId<<" on the worker "<<ptrBestWorker->name());
        		  return schedule_to(jobId, ptrBestWorker);
        	  }
        	  catch(const NoWorkerFoundException& ex1)
        	  {
        		  LOG(WARN, "No worker meets the requirements for the job " << jobId.str()<<" found!");
        		  ptr_comm_handler_->workerJobFailed("", jobId, "No worker meets the requirements for this job!");
        		  return false;
        	  }
          }
    }
    catch(const NoJobRequirements& )
    {
        // schedule to the first worker that requests a job
        schedule_anywhere(jobId);
        return true;
    }
  }
  else
  {
      //SDPA_LOG_DEBUG("Could not schedule job: no worker available: " << jobId);
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

	if( !numberOfWorkers() )
	{
		SDPA_LOG_WARN("No worker found. The job " << jobId<<" wasn't assigned to any worker. Try later!");
		throw NoWorkerFoundException();
	}
	else
	{
		lock_type lock(mtx_);
		schedule_with_constraints(jobId);
		cond_feed_workers.notify_one();
	}
}

// obsolete, only for testing purposes!
void SchedulerImpl::start_job(const sdpa::job_id_t &jobId)
{
	SDPA_LOG_DEBUG("Start the job "<<jobId.str());
}

void SchedulerImpl::schedule(const sdpa::job_id_t& jobId)
{
  DLOG(TRACE, "Schedule the job " << jobId.str());
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

const Worker::worker_id_t& SchedulerImpl::findAcknowlegedWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  return ptr_worker_man_->findAcknowlegedWorker(job_id);
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

  m_thread_run = boost::thread(boost::bind(&SchedulerImpl::run, this));
  m_thread_feed = boost::thread(boost::bind(&SchedulerImpl::feed_workers, this));


  SDPA_LOG_DEBUG("Scheduler thread started ...");
}

void SchedulerImpl::stop()
{
	bStopRequested = true;

  	m_thread_feed.interrupt();
  	DLOG(TRACE, "Feeding thread before join ...");
  	m_thread_feed.join();
  	DLOG(TRACE, "Feeding thread before join ...");

  	m_thread_run.interrupt();
  	DLOG(TRACE, "Scheduler thread before join ...");
  	m_thread_run.join();

  	DLOG(TRACE, "Scheduler thread joined ...");

  	if( jobs_to_be_scheduled.size() )
  	{
  		SDPA_LOG_WARN("There are still "<<jobs_to_be_scheduled.size()<<" jobs to be scheduled: " );
  		jobs_to_be_scheduled.print();
  	}

  	//ptr_comm_handler_ = NULL;
}

bool SchedulerImpl::post_request(const MasterInfo& masterInfo, bool force)
{
  if(!ptr_comm_handler_)
  {
    SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
    stop();
    return false;
  }

  bool bReqPosted = false;

  if(force || ( !ptr_comm_handler_->isTop()  &&  masterInfo.is_registered() ) )
  {
      bool bReqAllowed = ptr_comm_handler_->requestsAllowed();
      if(!bReqAllowed)
      {
          SDPA_LOG_DEBUG("The agent "<<ptr_comm_handler_->name()<<" is not allowed to post job requests!");
      }

      if( force || bReqAllowed )
      {
          ptr_comm_handler_->requestJob(masterInfo);
          //SDPA_LOG_DEBUG("The agent "<<ptr_comm_handler_->name()<<" has posted a new job request!");
          bReqPosted = true;
      }

  }

  return bReqPosted;
}

void SchedulerImpl::check_post_request()
{
	BOOST_FOREACH(sdpa::MasterInfo masterInfo, ptr_comm_handler_->getListMasterInfo())
	{
		if( !masterInfo.is_registered() )
		{
			SDPA_LOG_INFO("I'm not yet registered. Try to re-register ...");
			const unsigned long reg_timeout( ptr_comm_handler_->cfg().get<unsigned long>("registration_timeout", 10 *1000*1000) );
			SDPA_LOG_INFO("Wait " << reg_timeout/1000000 << "s before trying to re-register ...");
			boost::this_thread::sleep(boost::posix_time::microseconds(reg_timeout));

			ptr_comm_handler_->requestRegistration(masterInfo);
		}
		else
		{
			// post job request if number_of_jobs() < #registered workers +1
			if( useRequestModel() )
				post_request(masterInfo);
		}
	}
}

void SchedulerImpl::feed_workers()
{
	while(!bStopRequested)
	{
		lock_type lock(mtx_);
		cond_feed_workers.timed_wait(lock, m_timeout);

		sdpa::worker_id_list_t workerList;
		ptr_worker_man_->getWorkerListNotFull(workerList);

		if(!workerList.empty())
		{
			BOOST_FOREACH(const sdpa::worker_id_t& worker_id, workerList)
			{
				if(ptr_comm_handler_)
				{
					ptr_comm_handler_->serve_job(worker_id);
				}
				else
				{
					SDPA_LOG_WARN("Invalid communication handler");
				}
			}
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
			check_post_request(); // eventually, post a request to the master

			if( numberOfWorkers()>0 )
				forceOldWorkerJobsTermination();

			sdpa::job_id_t jobId   = jobs_to_be_scheduled.pop_and_wait(m_timeout);
			const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

			if( !pJob->isMasterJob() ) //it's a we job
			{
				// if it's an NRE just execute it!
				// Attention!: an NRE has no WorkerManager!!!!
				// or has an Worker Manager and the workers are threads
				if( numberOfWorkers()>0 ) //
				{
					try
					{
						schedule_remote(jobId);
						jobs_to_be_scheduled.print();
					}
					catch( const NoWorkerFoundException& ex)
					{
						SDPA_LOG_DEBUG("No valid worker found! Put the job "<<jobId.str()<<" into the common queue");
						// do so as when no preferences were set, just ignore them right now
						//schedule_anywhere(jobId);

						ptr_worker_man_->dispatchJob(jobId);
					}
				}
				else //  if has backends try to execute it
				{
					// just for testing
					if(ptr_comm_handler_->canRunTasksLocally())
					{
						DLOG(TRACE, "I have no workers, but I'm able to execute myself the job "<<jobId.str()<<" ...");
						execute(jobId);
					}
					else
					{
						//SDPA_LOG_DEBUG("no worker available, put the job back into the scheduler's queue!");
						if( !ptr_comm_handler_->canRunTasksLocally() )
						{
							jobs_to_be_scheduled.push(jobId);
							lock_type lock(mtx_);
							cond_workers_registered.wait(lock);
						}
						else
							execute(jobId);
					}

				} // else fail
			}
			else // it's a master job
				schedule_local(jobId);
		}
		catch(const JobNotFoundException& ex)
		{
			SDPA_LOG_WARN("Could not schedule the job "<<ex.job_id().str()<<". Job not found -> possible recovery inconsistency ...)");
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

void SchedulerImpl::execute(const sdpa::job_id_t& jobId)
{
	MLOG(TRACE, "executing activity: "<< jobId);
	const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);
	id_type act_id = pJob->id().str();

	execution_result_t result;
	encoded_type enc_act = pJob->description(); // assume that the NRE's workflow engine encodes the activity!!!

	if( !ptr_comm_handler_ )
	{
		LOG(ERROR, "nre scheduler does not have a comm-handler!");
		result_type output_fail;
		ptr_comm_handler_->workerJobFailed("", jobId, output_fail);
		return;
	}

	try
	{
		pJob->Dispatch();
		//result = m_worker_.execute(enc_act, pJob->walltime());
		boost::this_thread::sleep(boost::posix_time::milliseconds(2));
		result = std::make_pair(ACTIVITY_FINISHED, enc_act);
	}
	catch( const boost::thread_interrupted &)
	{
		std::string errmsg("could not execute activity: interrupted");
		SDPA_LOG_ERROR(errmsg);
		result = std::make_pair(ACTIVITY_FAILED, enc_act);
	}
	catch (const std::exception &ex)
	{
		std::string errmsg("could not execute activity: ");
		errmsg += std::string(ex.what());
		SDPA_LOG_ERROR(errmsg);
		result = std::make_pair(ACTIVITY_FAILED, enc_act);
	}

	// check the result state and invoke the NRE's callbacks
	if( result.first == ACTIVITY_FINISHED )
	{
		DLOG(TRACE, "activity finished: " << act_id);
		// notify the gui
		// and then, the workflow engine
		ptr_comm_handler_->workerJobFinished("", jobId, result.second);
	}
	else if( result.first == ACTIVITY_FAILED )
	{
		DLOG(TRACE, "activity failed: " << act_id);
		// notify the gui
		// and then, the workflow engine
		ptr_comm_handler_->workerJobFailed("", jobId, result.second);
	}
	else if( result.first == ACTIVITY_CANCELLED )
	{
		DLOG(TRACE, "activity cancelled: " << act_id);

		// notify the gui
		// and then, the workflow engine
		ptr_comm_handler_->workerJobCancelled("", jobId);
	}
	else
	{
		SDPA_LOG_ERROR("Invalid status of the executed activity received from the worker!");
		ptr_comm_handler_->workerJobFailed("", jobId, result.second);
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
  {
      SDPA_LOG_DEBUG("No job to be scheduled left!");
  }

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

	  // make sure that the job is erased from the scheduling queue
	  jobs_to_be_scheduled.erase( job_id );
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
	  lock_type lock(mtx_);
      ptr_worker_man_->deleteWorkerJob(worker_id, job_id);
      cond_feed_workers.notify_one();
  }
  catch(JobNotDeletedException const& ex1)
  {
	  SDPA_LOG_WARN("The job "<<job_id.str()<<" couldn't be found!");
      throw ex1;
  }
  catch(WorkerNotFoundException const &ex2 )
  {
      SDPA_LOG_WARN("The worker "<<worker_id<<" couldn't be found!");
      throw ex2;
  }
  catch(const std::exception& ex3 )
  {
      SDPA_LOG_WARN("Unexpected exception occurred when trying to delete the job "<<job_id.str()<<" from the worker "<<worker_id<<": "<< ex3.what() );
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

void SchedulerImpl::getWorkerList(std::list<std::string>& workerList)
{
	ptr_worker_man_->getWorkerList(workerList);
}

bool SchedulerImpl::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
{
	try
	{
		return ptr_worker_man_->addCapabilities(worker_id, cpbset);
	}
	catch(const std::exception& exc)
	{
		SDPA_LOG_ERROR("Exception occured when trying to add new capabilities to the worker "<<worker_id<<": "<<exc.what());
		throw exc;
	}
}

void SchedulerImpl::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException)
{
	ptr_worker_man_->removeCapabilities(worker_id, cpbset);
}

void SchedulerImpl::getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset)
{
	ptr_worker_man_->getCapabilities(ptr_comm_handler_->name(), cpbset);
}

void SchedulerImpl::getWorkerCapabilities(const sdpa::worker_id_t& worker_id, sdpa::capabilities_set_t& cpbset)
{
	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);
		cpbset = ptrWorker->capabilities();
	}
	catch(WorkerNotFoundException const &ex2 )
	{
		SDPA_LOG_ERROR("The worker "<<worker_id<<" could not be found!");
		cpbset = sdpa::capabilities_set_t();
	}
}

void SchedulerImpl::removeRecoveryInconsistencies()
{
	SDPA_LOG_INFO("Remove recovery inconsistencies!");
	std::list<JobQueue::iterator> listDirtyJobs;
	for(JobQueue::iterator it = jobs_to_be_scheduled.begin(); it != jobs_to_be_scheduled.end(); it++ )

	{
		try {
			const Job::ptr_t& pJob = ptr_comm_handler_->findJob(*it);
		}
		catch(const JobNotFoundException& ex)
		{
			listDirtyJobs.push_back(it);
		}
	}

	while(!listDirtyJobs.empty())
	{
		SDPA_LOG_INFO("Removing the job id "<<*listDirtyJobs.front()<<" from the scheduler's queue ...");
		jobs_to_be_scheduled.erase(listDirtyJobs.front());
		listDirtyJobs.pop_front();
	}
}

void SchedulerImpl::cancelWorkerJobs()
{
	ptr_worker_man_->cancelWorkerJobs(this);
}

void SchedulerImpl::planForCancellation(const Worker::worker_id_t& workerId, const sdpa::job_id_t& jobId)
{
	cancellation_list_.push_back(sdpa::worker_job_pair_t(workerId, jobId));
}

void SchedulerImpl::forceOldWorkerJobsTermination()
{
	// cannot recover the jobs produced by the workflow engine
	if(ptr_comm_handler_->hasWorkflowEngine())
	{
		sdpa::cancellation_list_t new_cancellation_list;
		while( !cancellation_list_.empty() )
		{
			worker_job_pair_t worker_job_pair = cancellation_list_.front();
			sdpa::worker_id_t workerId = worker_job_pair.first;
			sdpa::job_id_t jobId = worker_job_pair.second;

			try {
				SDPA_LOG_INFO("Tell the worker "<<workerId<<" to cancel the job "<<jobId);
				Worker::ptr_t pWorker = findWorker(workerId);

				CancelJobEvent::Ptr pEvtCancelJob (new CancelJobEvent( 	ptr_comm_handler_->name()
																		, workerId
																		, jobId
																		, "The master recovered after a crash!") );

				ptr_comm_handler_->sendEventToSlave(pEvtCancelJob);
			}
			catch (const WorkerNotFoundException& ex)
			{
				new_cancellation_list.push_back(worker_job_pair);
				//SDPA_LOG_WARN("Couldn't find the worker "<<workerId<<"(not registered yet)!");
			}

			cancellation_list_.pop_front();
		}

		cancellation_list_ = new_cancellation_list;
	}
}


Worker::worker_id_t SchedulerImpl::getWorkerId(unsigned int r)
{
	return ptr_worker_man_->getWorkerId(r);
}

void SchedulerImpl::setLastTimeServed(const worker_id_t& wid, const sdpa::util::time_type& servTime)
{
	ptr_worker_man_->setLastTimeServed(wid, servTime);
}
