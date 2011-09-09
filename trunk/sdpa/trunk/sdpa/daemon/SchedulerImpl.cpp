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

#include <cassert>
#include <boost/foreach.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;


SchedulerImpl::SchedulerImpl(sdpa::daemon::IComm* pCommHandler, bool bUseRequestModel )
  : ptr_worker_man_(new WorkerManager())
  , ptr_comm_handler_(pCommHandler)
  , SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name():"tests::sdpa::SchedulerImpl")
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_bUseRequestModel(bUseRequestModel)
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

void SchedulerImpl::addWorker( 	const Worker::worker_id_t& workerId,
								unsigned int capacity,
								const capabilities_set_t& cpbset,
								const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
  try {
      ptr_worker_man_->addWorker(workerId, capacity, cpbset, agent_uuid);
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
	// The result was succesfully delivered, so I can delete the job from the job map
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
		SDPA_LOG_ERROR("Cannot delete the worker "<<worker_id<<". Worker not found!");
		throw ex;
	}
	catch(JobNotFoundException const &ex)
	{
		SDPA_LOG_ERROR("Cannot re-schedule the job " << job_id << ". The job could not be found!");
		throw ex;
	}
	catch(JobNotDeletedException const & ex)
	{
		SDPA_LOG_ERROR("The job " << job_id << " could not be deleted: " << ex.what());
	}
	catch(const std::exception& ex) {
		SDPA_LOG_ERROR( "Could not re-schedule the job " << job_id << ": unexpected error!"<<ex.what() );
	}
}

void SchedulerImpl::reschedule( const Worker::worker_id_t & worker_id
                              , Worker::JobQueue* pQueue
                              )
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

    //pWorker->set_timedout();

    // The jobs submitted by the WE should have set a property
    // which indicates whether the daemon can safely re-schedule these activities or not (reason: ex global mem. alloc)

    // for each job in the queue, either re-schedule it, if is allowed
    // re_schedule( &pWorker->acknowledged() );
    // re_schedule( &pWorker->submitted() );
    // or declare it failed

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
	  //re_schedule(worker_id);

	  const Worker::ptr_t& pWorker = findWorker(worker_id);

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
  DMLOG(TRACE, "Called schedule_local ...");

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
	  DMLOG(TRACE, "Submit the workflow attached to the job "<<wf_id<<" to WE. Workflow description follows: ");

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
	// should have a monitoring thread that detects the timedout nodes
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
	return schedule_to(jobId, workerId);
}

void SchedulerImpl::delete_job (sdpa::job_id_t const & job)
{
  LOG(TRACE, "removing job " << job << " from the scheduler....");
  ptr_worker_man_->delete_job(job);
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
      DMLOG(TRACE, "Check if there are requirements specified for the job "<<jobId.str()<<"  ... ");

      try
      {
    	  const requirement_list_t job_req = ptr_comm_handler_->getJobRequirements(jobId);

          // no preferences specified
          if( job_req.empty() )
          {
        	  // schedule to the first worker that requests a job
              schedule_anywhere(jobId);
              return true;
          }
          else // there are requirements specified for that job
          {
        	  try
        	  {
        		  // first round: get the list of all workers for which the mandatory requirements are matching the capabilities
        		  Worker::ptr_t ptrBestWorker = ptr_worker_man_->getBestMatchingWorker(job_req);

        		  // effectively assign the job to that worker

        		  // schedule the job to that one
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

  if( !numberOfWorkers() )
  {
      SDPA_LOG_WARN("No worker found. The job " << jobId<<" wasn't assigned to any worker. Try later!");
      throw NoWorkerFoundException();
  }
  else
      schedule_with_constraints(jobId);
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

const Worker::worker_id_t&
SchedulerImpl::findAcknowlegedWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
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

  if(force || ( !ptr_comm_handler_->is_orchestrator()  &&  masterInfo.is_registered() ) )
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
			const unsigned long reg_timeout( ptr_comm_handler_->cfg().get<unsigned long>("registration_timeout", 1 *1000*1000) );
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
	// for any worker take a job from its pending queue, a
	// std::list<std::string> listAvailWorkers;
	// ptr_worker_man_->getListAvailWorkers(listAvailWorkers);

	// start here with the last served worker
	// BOOST_FOREACH(sdpa::worker_id_t worker_id, listAvailWorkers)

	// as long as there are still jobs into the central queue
	// and there are still workers not fully loaded do feed workers
	// i.e take jobs from the pending queues (or from the common queue )
	// and send them

	// attention: some jobs may be rejected i.e. an error message of type SDPA_EJOBREJECTED
	// is triggered with the reason = job_id -> handle this message -> re-schedule the
	// job: don't forget to update the job status -> reverse state or create a new job
	// with the same id after deleting the original one


    try {

    	sdpa::worker_id_t worker_id = ptr_worker_man_->getLeastLoadedWorker();
    	Worker::ptr_t pWorker(findWorker(worker_id));

		if(ptr_comm_handler_)
		{
			ptr_comm_handler_->serve_job(worker_id);
		}
		else
		{
			SDPA_LOG_WARN("Invalid communication handler");
		}

    }
    catch(const NoWorkerFoundException& ) {
      //SDPA_LOG_WARN("No worker found!");
    }
    catch(const AllWorkersFullException&) {
      LOG_EVERY_N(WARN, 500, "FIXME: all workers are full -> don't run the scheduler if not needed...");
    }
    catch (std::exception const& ex) {
    	SDPA_LOG_ERROR("An unexpected exception occurred when attempting to feed the workers");
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
    		  feed_workers(); //eventually, feed some workers

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

void SchedulerImpl::getWorkerList(std::list<std::string>& workerList)
{
	ptr_worker_man_->getWorkerList(workerList);
}

void SchedulerImpl::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException)
{
	ptr_worker_man_->addCapabilities(worker_id, cpbset);
}

void SchedulerImpl::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException)
{
	ptr_worker_man_->removeCapabilities(worker_id, cpbset);
}

void SchedulerImpl::getCapabilities(sdpa::capabilities_set_t& cpbset)
{
	ptr_worker_man_->getCapabilities(cpbset);
}
