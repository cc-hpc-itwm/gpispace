//tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/SchedulerBase.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa {
   namespace daemon {

SchedulerBase::SchedulerBase(GenericDaemon* pCommHandler)
  : _worker_manager()
  , ptr_comm_handler_ ( pCommHandler
                      ? pCommHandler
                      : throw std::runtime_error
                        ("SchedulerBase ctor with NULL ptr_comm_handler")
                      )
  , SDPA_INIT_LOGGER (ptr_comm_handler_->name())
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
                                const sdpa::worker_id_t& agent_uuid )
{
  lock_type lock(mtx_);
  _worker_manager.addWorker(workerId, capacity, cpbset, agent_rank, agent_uuid);
  cond_workers_registered.notify_all();
  cond_feed_workers.notify_one();
}

void SchedulerBase::rescheduleWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
{
  lock_type lock(mtx_);
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
  lock_type lock(mtx_);
  while( !workerJobList.empty() ) {
      sdpa::job_id_t jobId = workerJobList.front();
      DMLOG (TRACE, "Re-scheduling the job "<<jobId.str()<<" ... ");
      rescheduleWorkerJob(worker_id, jobId);
      workerJobList.pop_front();
  }
}

void SchedulerBase::deleteWorker( const Worker::worker_id_t& worker_id )
{
  lock_type lock(mtx_);
  try {

    // mark the worker dirty -> don't take it in consideration for re-scheduling
    const Worker::ptr_t pWorker = findWorker(worker_id);
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
  lock_type lock(mtx_);
  Job* pJob = ptr_comm_handler_->findJob(jobId);
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
      sdpa::events::JobFailedEvent::Ptr pEvtJobFailed
            (new sdpa::events::JobFailedEvent(  m_agent_name
                                  , m_agent_name
                                  , jobId
                                  , fhg::error::UNEXPECTED_ERROR
                                  , ex.what()
                                   ));

      ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
    }
  }
  else
  {
    sdpa::events::JobFailedEvent::Ptr pEvtJobFailed(new sdpa::events::JobFailedEvent(m_agent_name
                                                         , m_agent_name
                                                         , jobId
                                                         , fhg::error::UNEXPECTED_ERROR
                                                         , "job could not be found"
                                                         ));

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
}

void SchedulerBase::enqueueJob(const sdpa::job_id_t& jobId)
{
  pending_jobs_queue_.push(jobId);
}

Worker::ptr_t SchedulerBase::findWorker(const Worker::worker_id_t& worker_id )
{
  return _worker_manager.findWorker(worker_id);
}

bool SchedulerBase::hasWorker(const Worker::worker_id_t& worker_id) const
{
  return _worker_manager.hasWorker(worker_id);
}

const boost::optional<Worker::worker_id_t> SchedulerBase::findSubmOrAckWorker(const sdpa::job_id_t& job_id) const
{
  return _worker_manager.findSubmOrAckWorker(job_id);
}

void SchedulerBase::getListNotFullWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  _worker_manager.getListNotFullWorkers(workerList);
}

sdpa::worker_id_t SchedulerBase::findSuitableWorker
  (const job_requirements_t& job_reqs, sdpa::worker_id_list_t& listAvailWorkers)
{
  lock_type lock(mtx_);
  sdpa::worker_id_t matchingWorkerId;

  if (listAvailWorkers.empty())
  {
    return matchingWorkerId;
  }

  if (job_reqs.empty())
  {
    matchingWorkerId = listAvailWorkers.front();
    listAvailWorkers.pop_front();
  }
  else
  {
    try {
      matchingWorkerId = _worker_manager.getBestMatchingWorker(job_reqs, listAvailWorkers);
      sdpa::worker_id_list_t::iterator it = std::find(listAvailWorkers.begin(), listAvailWorkers.end(), matchingWorkerId);
      listAvailWorkers.erase(it);
    }
    catch(NoWorkerFoundException& exc) {
    }
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
    try
    {
      sdpa::job_id_t jobId = pending_jobs_queue_.pop_and_wait();

      if( numberOfWorkers()>0 ) {
          schedule(jobId);
      }
      else {
          enqueueJob(jobId);
          lock_type lock(mtx_);
          cond_workers_registered.wait(lock);
      }
    }
    catch ( const std::exception &ex )
    {
        MLOG(ERROR, "exception in scheduler thread: " << ex.what());
        throw;
    }
  }
}

void SchedulerBase::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id)
{
  lock_type lock(mtx_);
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
  catch(WorkerNotFoundException const &ex2)
  {
    throw ex2;
  }
}

void SchedulerBase::deleteWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t &jobId )
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
  lock_type lock(mtx_);
  return pending_jobs_queue_.has_item(job_id)|| _worker_manager.has_job(job_id);
}

void SchedulerBase::getWorkerList(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  _worker_manager.getWorkerList(workerList);
}

bool SchedulerBase::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
{
  lock_type lock(mtx_);
  if(_worker_manager.addCapabilities(worker_id, cpbset))
  {
    cond_feed_workers.notify_one();
    return true;
  }
  else
    return false;
}

void SchedulerBase::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
{
  _worker_manager.removeCapabilities(worker_id, cpbset);
}

void SchedulerBase::getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset)
{
  _worker_manager.getCapabilities(m_agent_name, cpbset);
}

void SchedulerBase::getWorkerCapabilities(const sdpa::worker_id_t& worker_id, sdpa::capabilities_set_t& cpbset)
{
  lock_type lock(mtx_);
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

}}
