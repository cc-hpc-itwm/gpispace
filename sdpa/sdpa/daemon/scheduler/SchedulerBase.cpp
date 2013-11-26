/*
 * =====================================================================================
 *
 *       Filename:  SchedulerBase.hpp
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

#include <sdpa/daemon/scheduler/SchedulerBase.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SchedulerBase::SchedulerBase(sdpa::daemon::IAgent* pCommHandler)
  : _worker_manager()
  , ptr_comm_handler_ ( pCommHandler
                      ? pCommHandler
                      : throw std::runtime_error
                        ("SchedulerBase ctor with NULL ptr_comm_handler")
                      )
  , SDPA_INIT_LOGGER (ptr_comm_handler_->name())
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_agent_name (ptr_comm_handler_->name())
{}

void SchedulerBase::start_threads()
{
  m_thread_run = boost::thread (&SchedulerBase::run, this);
  m_thread_feed = boost::thread (&SchedulerBase::feedWorkers, this);
}

SchedulerBase::~SchedulerBase()
{
  m_thread_run.interrupt();
  m_thread_feed.interrupt();

  if (m_thread_run.joinable() )
    m_thread_run.join();

  if (m_thread_feed.joinable() )
    m_thread_feed.join();
}


void SchedulerBase::addWorker(  const Worker::worker_id_t& workerId,
                                const boost::optional<unsigned int>& capacity,
                                const capabilities_set_t& cpbset,
                                const unsigned int& agent_rank,
                                const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
  _worker_manager.addWorker(workerId, capacity, cpbset, agent_rank, agent_uuid);
  cond_workers_registered.notify_all();
  cond_feed_workers.notify_one();
}

void SchedulerBase::rescheduleWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
{
  try
  {
      // delete it from the worker's queues
      Worker::ptr_t pWorker = findWorker(worker_id);
      pWorker->deleteJob(job_id);
  }
  catch (const WorkerNotFoundException& ex)
  {
      SDPA_LOG_WARN("Cannot find the worker "<<worker_id);
  }
  catch(JobNotDeletedException const & ex)
  {
      SDPA_LOG_WARN("The job " << job_id << " could not be deleted: " << ex.what());
  }

  rescheduleJob(job_id);
}

void SchedulerBase::reschedule( const Worker::worker_id_t & worker_id, sdpa::job_id_list_t& workerJobList )
{
      while( !workerJobList.empty() ) {
          sdpa::job_id_t jobId = workerJobList.front();
	  DMLOG (TRACE, "Re-scheduling the job "<<jobId.str()<<" ... ");
	  rescheduleWorkerJob(worker_id, jobId);
	  workerJobList.pop_front();
      }
}

void SchedulerBase::deleteWorker( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
  // first re-schedule the work:
  // inspect all queues and re-schedule each job
  try {

    // mark the worker dirty -> don't take it in consideration for re-scheduling
    const Worker::ptr_t& pWorker = findWorker(worker_id);
    pWorker->set_disconnected(true);

    sdpa::job_id_list_t workerJobList(_worker_manager.getJobListAndCleanQueues(pWorker));
    reschedule(worker_id, workerJobList);

    if( !workerJobList.empty() )
    {
        SDPA_LOG_WARN( "The worker " << worker_id << " has still has assigned jobs!");
    }

    // delete the worker from the worker map
    _worker_manager.deleteWorker(worker_id);
  }
  catch (const WorkerNotFoundException& ex)
  {
      SDPA_LOG_ERROR("Cannot delete the worker "<<worker_id<<". Worker not found!");
      throw ex;
  }
}

void SchedulerBase::delete_job (sdpa::job_id_t const & job)
{
  SDPA_LOG_DEBUG("removing job " << job << " from the scheduler's queue ....");
  if (pending_jobs_queue_.erase(job))
  {
    SDPA_LOG_DEBUG("removed job from the central queue...");
  }
  else
    _worker_manager.deleteJob(job);
}

void SchedulerBase::schedule(const sdpa::job_id_t& jobId)
{
  const Job::ptr_t pJob = ptr_comm_handler_->findJob(jobId);
  if(pJob)
  {
    try {
        _worker_manager.dispatchJob(jobId);
        // put the job into the running state
        pJob->Dispatch();
        cond_feed_workers.notify_one();
    }
    catch (std::exception const & ex)
    {
      //send a JobFailed event
      sdpa::job_result_t result(ex.what());

      JobFailedEvent::Ptr pEvtJobFailed
            (new JobFailedEvent(  m_agent_name
                                  , m_agent_name
                                  , jobId
                                  , result
                                  , fhg::error::UNEXPECTED_ERROR
                                  , ex.what()
                                   ));

      ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
    }
  }
  else
  {
    sdpa::job_result_t result("Inexsitent job: "+jobId.str());

    JobFailedEvent::Ptr pEvtJobFailed(new JobFailedEvent(m_agent_name
                                                         , m_agent_name
                                                         , jobId
                                                         , result
                                                         , fhg::error::UNEXPECTED_ERROR
                                                         , "job could not be found"
                                                         ));

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
}

void SchedulerBase::enqueueJob(const sdpa::job_id_t& jobId)
{
  //DMLOG(TRACE, "Schedule the job " << jobId.str());
  pending_jobs_queue_.push(jobId);
}

const Worker::ptr_t& SchedulerBase::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
  return _worker_manager.findWorker(worker_id);
}

const Worker::worker_id_t& SchedulerBase::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  return _worker_manager.findWorker(job_id);
}

const Worker::worker_id_t& SchedulerBase::findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  return _worker_manager.findSubmOrAckWorker(job_id);
}

void SchedulerBase::getListNotFullWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  _worker_manager.getListNotFullWorkers(workerList);
}

sdpa::worker_id_t SchedulerBase::findSuitableWorker(const job_requirements_t& job_reqs, sdpa::worker_id_list_t& listAvailWorkers)
{
  lock_type lock(mtx_);
  sdpa::worker_id_t matchingWorkerId;

  try {
      matchingWorkerId = _worker_manager.getBestMatchingWorker(job_reqs, listAvailWorkers);
      sdpa::worker_id_list_t::iterator it = std::find(listAvailWorkers.begin(), listAvailWorkers.end(), matchingWorkerId);
      listAvailWorkers.erase(it);
  }
  catch(NoWorkerFoundException& exc) {
  }

  return matchingWorkerId;
}

void SchedulerBase::feedWorkers()
{
  for (;;)
  {
    lock_type lock (mtx_);
    cond_feed_workers.wait (lock);

    if (ptr_comm_handler_->hasJobs())
    {
      assignJobsToWorkers();
    }
  }
}

void SchedulerBase::run()
{
  for (;;)
  {
    boost::this_thread::interruption_point();

    try
    {
      sdpa::job_id_t jobId = pending_jobs_queue_.pop_and_wait(m_timeout);

      if( numberOfWorkers()>0 ) {
          schedule(jobId);
      }
      else {
          enqueueJob(jobId);
          // mark the job as stalled
          ptr_comm_handler_->pause(jobId);
          lock_type lock(mtx_);
          cond_workers_registered.wait(lock);
      }
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

void SchedulerBase::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw( WorkerNotFoundException, JobNotFoundException)
{
  DLOG(TRACE, "Acknowledge the job "<<job_id.str());
  try {
    // make sure that the job is erased from the scheduling queue

	// don't need this!
	//jobs_to_be_scheduled.erase( job_id );
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
  catch(WorkerNotFoundException const &ex2)
  {
    throw ex2;
  }
}

void SchedulerBase::deleteWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t &jobId ) throw (JobNotDeletedException, WorkerNotFoundException)
{
  try {
    lock_type lock(mtx_);

    ///jobs_to_be_scheduled.erase( job_id );
    // check if there is an allocation list for this job
    // (assert that the head of this list id worker_id!)
    // free all the workers in this list, i.e. mark them as not reserved
    _worker_manager.deleteWorkerJob(worker_id, jobId);
    cond_feed_workers.notify_one();
  }
  catch(JobNotDeletedException const& ex1)
  {
    SDPA_LOG_WARN("The job "<<jobId.str()<<" couldn't be found!");
    throw ex1;
  }
  catch(WorkerNotFoundException const &ex2 )
  {
    SDPA_LOG_WARN("The worker "<<worker_id<<" couldn't be found!");
    throw ex2;
  }
}

bool SchedulerBase::has_job(const sdpa::job_id_t& job_id)
{
  return pending_jobs_queue_.has_item(job_id)|| _worker_manager.has_job(job_id);
}

void SchedulerBase::getWorkerList(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  _worker_manager.getWorkerList(workerList);
}

bool SchedulerBase::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
{
  return _worker_manager.addCapabilities(worker_id, cpbset);
}

void SchedulerBase::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException)
{
  _worker_manager.removeCapabilities(worker_id, cpbset);
}

void SchedulerBase::getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset)
{
  _worker_manager.getCapabilities(m_agent_name, cpbset);
}

void SchedulerBase::getWorkerCapabilities(const sdpa::worker_id_t& worker_id, sdpa::capabilities_set_t& cpbset)
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

void SchedulerBase::schedule_first(const sdpa::job_id_t& jid)
{
  _worker_manager.common_queue_.push_front(jid);
}
