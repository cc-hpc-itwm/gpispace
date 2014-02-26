// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa {
   namespace daemon {

CoallocationScheduler::CoallocationScheduler
    (GenericDaemon* pCommHandler, bool TEST_without_threads)
  : ptr_comm_handler_ ( pCommHandler
                      ? pCommHandler
                      : throw std::runtime_error
                        ("CoallocationScheduler ctor with NULL ptr_comm_handler")
                      )
  , _logger (fhg::log::Logger::get (ptr_comm_handler_->name()))
  , _worker_manager()
{
  if (!TEST_without_threads)
  {
    m_thread_run = boost::thread (&CoallocationScheduler::run, this);
    m_thread_feed = boost::thread (&CoallocationScheduler::feedWorkers, this);
  }
}

CoallocationScheduler::~CoallocationScheduler()
{
  m_thread_run.interrupt();
  m_thread_feed.interrupt();

  if (m_thread_run.joinable() )
    m_thread_run.join();

  if (m_thread_feed.joinable() )
    m_thread_feed.join();
}


bool CoallocationScheduler::addWorker(  const Worker::worker_id_t& workerId,
                                const boost::optional<unsigned int>& capacity,
                                const capabilities_set_t& cpbset )
{
  lock_type lock(mtx_);
  const bool ret (_worker_manager.addWorker(workerId, capacity, cpbset));
  cond_workers_registered.notify_all();
  cond_feed_workers.notify_one();
  return ret;
}

void CoallocationScheduler::rescheduleWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
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
  }

  rescheduleJob(job_id);
}

void CoallocationScheduler::reschedule( const Worker::worker_id_t & worker_id, sdpa::job_id_list_t& workerJobList )
{
  lock_type lock(mtx_);
  while( !workerJobList.empty() ) {
      sdpa::job_id_t jobId = workerJobList.front();
      rescheduleWorkerJob(worker_id, jobId);
      workerJobList.pop_front();
  }
}

void CoallocationScheduler::deleteWorker( const Worker::worker_id_t& worker_id )
{
  lock_type lock(mtx_);
    // mark the worker dirty -> don't take it in consideration for re-scheduling
    const Worker::ptr_t pWorker = findWorker(worker_id);
    pWorker->set_disconnected(true);

    sdpa::job_id_list_t workerJobList(_worker_manager.getJobListAndCleanQueues(pWorker));
    reschedule(worker_id, workerJobList);

    // delete the worker from the worker map
    _worker_manager.deleteWorker(worker_id);
}

void CoallocationScheduler::delete_job (sdpa::job_id_t const & job)
{
  if (!pending_jobs_queue_.erase(job))
  {
    _worker_manager.deleteJob(job);
  }
}

void CoallocationScheduler::schedule(const sdpa::job_id_t& jobId)
{
  lock_type lock(mtx_);
  Job* pJob = ptr_comm_handler_->findJob(jobId);
  if(!pJob)
  {
    throw std::runtime_error ("tried scheduling non-existent job");
  }
  schedule (pJob);
}
void CoallocationScheduler::schedule (Job* pJob)
{
  _worker_manager.dispatchJob(pJob->id());
  cond_feed_workers.notify_one();
}

void CoallocationScheduler::enqueueJob(const sdpa::job_id_t& jobId)
{
  pending_jobs_queue_.push(jobId);
}

Worker::ptr_t CoallocationScheduler::findWorker(const Worker::worker_id_t& worker_id )
{
  return _worker_manager.findWorker(worker_id);
}

bool CoallocationScheduler::hasWorker(const Worker::worker_id_t& worker_id) const
{
  return _worker_manager.hasWorker(worker_id);
}

const boost::optional<Worker::worker_id_t> CoallocationScheduler::findSubmOrAckWorker(const sdpa::job_id_t& job_id) const
{
  return _worker_manager.findSubmOrAckWorker(job_id);
}

void CoallocationScheduler::getListNotFullWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  _worker_manager.getListNotFullWorkers(workerList);
}

boost::optional<sdpa::worker_id_t> CoallocationScheduler::findSuitableWorker
  (const job_requirements_t& job_reqs, const sdpa::worker_id_list_t& listAvailWorkers)
{
  lock_type lock(mtx_);

  if (listAvailWorkers.empty())
  {
    return boost::none;
  }

  return job_reqs.empty()
    ? listAvailWorkers.front()
    : _worker_manager.getBestMatchingWorker (job_reqs, listAvailWorkers);
}

void CoallocationScheduler::feedWorkers()
{
  for (;;)
  {
    lock_type lock (mtx_);
    cond_feed_workers.wait (lock);

    assignJobsToWorkers();
  }
}

void CoallocationScheduler::run()
{
  for (;;)
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
}

void CoallocationScheduler::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id)
{
  lock_type lock(mtx_);
    Worker::ptr_t ptrWorker = findWorker(worker_id);

    //put the job into the Running state: do this in acknowledge!
    if( !ptrWorker->acknowledge(job_id) )
      throw JobNotFoundException();
}

void CoallocationScheduler::deleteWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t &jobId )
{
    lock_type lock(mtx_);

    ///jobs_to_be_scheduled.erase( job_id );
    // check if there is an allocation list for this job
    // (assert that the head of this list id worker_id!)
    // free all the workers in this list, i.e. mark them as not reserved
    _worker_manager.deleteWorkerJob(worker_id, jobId);
    cond_feed_workers.notify_one();
}

bool CoallocationScheduler::has_job(const sdpa::job_id_t& job_id)
{
  lock_type lock(mtx_);
  return pending_jobs_queue_.has_item(job_id)|| _worker_manager.has_job(job_id);
}

bool CoallocationScheduler::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
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

void CoallocationScheduler::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
{
  _worker_manager.removeCapabilities(worker_id, cpbset);
}

void CoallocationScheduler::getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset)
{
  _worker_manager.getCapabilities(cpbset);
}

sdpa::capabilities_set_t CoallocationScheduler::getWorkerCapabilities
  (const sdpa::worker_id_t& worker_id)
{
  lock_type lock(mtx_);
  try {
      Worker::ptr_t ptrWorker = findWorker(worker_id);
      return ptrWorker->capabilities();
  }
  catch(WorkerNotFoundException const &ex2 )
  {
      return sdpa::capabilities_set_t();
  }
}

void CoallocationScheduler::schedule_first(const sdpa::job_id_t& jid)
{
  _worker_manager.common_queue_.push_front(jid);
}

void CoallocationScheduler::assignJobsToWorkers()
{
  lock_type lock(mtx_);

  if(!schedulingAllowed())
    return;

  // replace this with the list of workers not reserved
  //getListNotFullWorkers(listAvailWorkers);
  sdpa::worker_id_list_t listAvailWorkers;
  _worker_manager.getListWorkersNotReserved(listAvailWorkers);

  // check if there are jobs that can already be scheduled on
  // these workers
  std::list<sdpa::job_id_t> nonmatching_jobs_queue;

  // iterate over all jobs and see if there is one that prefers
  while(schedulingAllowed() && !listAvailWorkers.empty())
  {
    sdpa::job_id_t jobId(nextJobToSchedule());

    const job_requirements_t job_reqs
      (ptr_comm_handler_->getJobRequirements (jobId));

    unsigned long nReqWorkers (job_reqs.numWorkers());
    const boost::optional<sdpa::worker_id_t> matchingWorkerId
      (findSuitableWorker(job_reqs, listAvailWorkers));

    if (matchingWorkerId) // matching found
    {
      listAvailWorkers.erase ( std::find ( listAvailWorkers.begin()
                                         , listAvailWorkers.end()
                                         , *matchingWorkerId
                                         )
                             );

        lock_type lock(mtx_alloc_table_);
        reserveWorker(jobId, *matchingWorkerId, nReqWorkers);

        // attention: what to do if job_reqs.n_workers_req > total number of registered workers?
        // if all the required resources were acquired, mark the job as submitted
        Reservation* pReservation(allocation_table_[jobId]);

        if( pReservation->acquired() )
        {
            sdpa::worker_id_list_t list_reserved_workers = pReservation->getWorkerList();
            // check if the reservation is valid
            sdpa::worker_id_list_t list_invalid_workers = checkReservationIsValid(*pReservation);
            if(list_invalid_workers.empty())
            {
              // serve the same job to all reserved workers!!!!

              _worker_manager.markJobSubmitted(pReservation->getWorkerList(), jobId);
              ptr_comm_handler_->serveJob(list_reserved_workers, jobId);
            }
            else
            {
              // delete the invalid workers
              BOOST_FOREACH(const Worker::worker_id_t& wid, list_reserved_workers)
              {
                pReservation->delWorker(wid);
              }

              schedule_first(jobId);
            }
        }
        else
        {
            schedule_first(jobId);
        }
    }
    else // put it back into the common queue
    {
        nonmatching_jobs_queue.push_back(jobId);
    }
  }

  BOOST_REVERSE_FOREACH (const sdpa::job_id_t& id, nonmatching_jobs_queue)
  {
    schedule_first (id);
  }
}

sdpa::worker_id_list_t CoallocationScheduler::checkReservationIsValid(const Reservation& res)
{
  lock_type lock(mtx_);
  sdpa::worker_id_list_t list_del_workers;
  sdpa::worker_id_list_t res_worker_list(res.getWorkerList());
  BOOST_FOREACH(const Worker::worker_id_t& wid, res_worker_list)
  {
    if(!hasWorker(wid))
      list_del_workers.push_back(wid);
  }
  return list_del_workers;
}

void CoallocationScheduler::rescheduleJob(const sdpa::job_id_t& job_id )
{
  Job* pJob = ptr_comm_handler_->findJob(job_id);
  if(pJob)
  {
    if(!sdpa::status::is_terminal (pJob->getStatus())) {
      releaseReservation(job_id);
      pJob->Reschedule(); // put the job back into the pending state
      schedule (pJob);
    }
  }
  else //(JobNotFoundException const &ex)
  {
    LLOG (WARN, _logger, "Cannot re-schedule the job " << job_id << ". The job could not be found!");
  }
}

void CoallocationScheduler::reserveWorker(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& matchingWorkerId, const size_t& cap)
{
  lock_type lock_table(mtx_alloc_table_);
  _worker_manager.reserveWorker(matchingWorkerId);
  // allocate this worker to the job with the jobId

  allocation_table_t::iterator it(allocation_table_.find(jobId));
  if(it==allocation_table_.end()) {
      Reservation*  pReservation(new Reservation(jobId, cap));
      allocation_table_t::value_type pairJobRes(jobId, pReservation);
      allocation_table_.insert(pairJobRes);
  }

  allocation_table_[jobId]->addWorker(matchingWorkerId);
}

void CoallocationScheduler::releaseReservation(const sdpa::job_id_t& jobId)
{
  lock_type lock_table(mtx_alloc_table_);

  // if the status is not terminal
  try {
      allocation_table_t::const_iterator it = allocation_table_.find(jobId);

      // if there are allocated resources
      if(it==allocation_table_.end()) {
          return;
      }

      lock_type lock_worker (mtx_);
      Reservation* pReservation(allocation_table_[jobId]);
      worker_id_list_t listWorkers(pReservation->getWorkerList());
      BOOST_FOREACH (sdpa::worker_id_t const& workerId, listWorkers)
      {
        try
        {
          findWorker(workerId)->free();
        }
        catch (WorkerNotFoundException const& ex)
        {
        }
      }

      delete allocation_table_[jobId];
      allocation_table_[jobId] = NULL;
      allocation_table_.erase(jobId);
  }
  catch(JobNotFoundException const& ex2)
  {
  }
}

sdpa::worker_id_list_t CoallocationScheduler::getListAllocatedWorkers(const sdpa::job_id_t& jobId)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jobId);
  if(it!=allocation_table_.end())
    return it->second->getWorkerList();
  else
    return sdpa::worker_id_list_t();
}

void CoallocationScheduler::workerFinished(const worker_id_t& wid, const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    it->second->workerFinished(wid);
  else
    throw WorkerNotFoundException();
}

void CoallocationScheduler::workerFailed(const worker_id_t& wid, const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    it->second->workerFailed(wid);
  else
    throw WorkerNotFoundException();
}

void CoallocationScheduler::workerCanceled(const worker_id_t& wid, const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    it->second->workerCanceled(wid);
  else
    throw WorkerNotFoundException();
}

bool CoallocationScheduler::allPartialResultsCollected(const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    return it->second->allWorkersTerminated();
  else
    throw JobNotFoundException();
}

bool CoallocationScheduler::groupFinished(const sdpa::job_id_t& jid)
{
  lock_type lock(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    return it->second->allGroupTasksFinishedSuccessfully();
  else
    throw JobNotFoundException();
}

}}
