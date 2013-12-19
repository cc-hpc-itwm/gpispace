// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa {
  namespace daemon {

CoallocationScheduler::CoallocationScheduler(GenericDaemon* pCommHandler)
  : SchedulerBase(pCommHandler)
{}

void CoallocationScheduler::assignJobsToWorkers()
{
  lock_type lock(mtx_);
  sdpa::worker_id_list_t listAvailWorkers;

  if(!schedulingAllowed())
    return;

  // replace this with the list of workers not reserved
  //getListNotFullWorkers(listAvailWorkers);
  getListNotAllocatedWorkers(listAvailWorkers);

  // check if there are jobs that can already be scheduled on
  // these workers
  JobQueue nonmatching_jobs_queue;

  // iterate over all jobs and see if there is one that prefers
  while(schedulingAllowed() && !listAvailWorkers.empty())
  {
    sdpa::job_id_t jobId(nextJobToSchedule());

    const job_requirements_t job_reqs
      (ptr_comm_handler_->getJobRequirements (jobId));

    long nReqWorkers (job_reqs.numWorkers());
    const sdpa::worker_id_t matchingWorkerId
      (findSuitableWorker(job_reqs, listAvailWorkers));

    if( !matchingWorkerId.empty() ) // matching found
    {
        lock_type lock(mtx_alloc_table_);
        reserveWorker(jobId, matchingWorkerId, nReqWorkers);

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
              DMLOG(DEBUG, "A reservation for the job "<<jobId<<" has been acquired! List of assigned workers: "<<list_reserved_workers);
              // serve the same job to all reserved workers!!!!

              ptr_comm_handler_->resume(jobId);
              ptr_comm_handler_->serveJob(list_reserved_workers, jobId);
              _worker_manager.markJobSubmitted(pReservation->getWorkerList(), jobId);
            }
            else
            {
              ptr_comm_handler_->pause(jobId);
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
            ptr_comm_handler_->pause(jobId);
            schedule_first(jobId);
        }
    }
    else // put it back into the common queue
    {
        ptr_comm_handler_->pause(jobId);
        nonmatching_jobs_queue.push(jobId);
    }
  }

  while(!nonmatching_jobs_queue.empty())
    schedule_first(nonmatching_jobs_queue.pop_back());
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
    if(!pJob->completed()) {
      releaseReservation(job_id);
      pJob->Reschedule(this); // put the job back into the pending state
    }
  }
  else //(JobNotFoundException const &ex)
  {
      SDPA_LOG_WARN("Cannot re-schedule the job " << job_id << ". The job could not be found!");
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
          DLOG(WARN, "No reservation was found for the job "<<jobId);
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
          DLOG(WARN, "The worker "<<workerId<<" was not found, it was already released!");
        }
      }

      delete allocation_table_[jobId];
      allocation_table_[jobId] = NULL;
      allocation_table_.erase(jobId);
  }
  catch(JobNotFoundException const& ex2)
  {
      DLOG(WARN, "The job "<<jobId<<" was not found!");
  }
}

void CoallocationScheduler::getListNotAllocatedWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  _worker_manager.getListWorkersNotReserved(workerList);
}

sdpa::job_id_t CoallocationScheduler::getAssignedJob(const sdpa::worker_id_t& wid)
{
  lock_type lock(mtx_alloc_table_);

  allocation_table_t::iterator it = allocation_table_.begin();
  while(it != allocation_table_.end())
  {
      if(it->second->hasWorker(wid))
        return it->first;
      else
        it++;
  }

  return job_id_t("");
}

void CoallocationScheduler::checkAllocations()
{
  lock_type lock(mtx_alloc_table_);
  typedef std::map<worker_id_t,int> worker_cnt_map_t;
  worker_cnt_map_t worker_cnt_map;
  worker_id_list_t worker_list;
  getWorkerList(worker_list);

  BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
  {
    worker_cnt_map.insert(worker_cnt_map_t::value_type(worker_id, 0));
  }

  BOOST_FOREACH(const allocation_table_t::value_type& pairJLW, allocation_table_)
  {
    worker_id_list_t workerList(pairJLW.second->getWorkerList());
    BOOST_FOREACH(const sdpa::worker_id_t& wid, workerList)
    {
      worker_cnt_map[wid]++;
      if(worker_cnt_map[wid]>1)
        {
          DLOG(FATAL, "Error! The worker "<<wid<<" was allocated to two different jobs!");
          throw;
        }
    }
  }

  std::ostringstream oss;
  BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
  {
    oss<<worker_id<<":"<<worker_cnt_map[worker_id]<<" ";
  }
  DLOG(TRACE, oss.str());
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
    throw WorkerNotFoundException(wid);
}

void CoallocationScheduler::workerFailed(const worker_id_t& wid, const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    it->second->workerFailed(wid);
  else
    throw WorkerNotFoundException(wid);
}

void CoallocationScheduler::workerCanceled(const worker_id_t& wid, const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    it->second->workerCanceled(wid);
  else
    throw WorkerNotFoundException(wid);
}

bool CoallocationScheduler::allPartialResultsCollected(const job_id_t& jid)
{
  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    return it->second->allWorkersTerminated();
  else
    throw JobNotFoundException(jid);
}

bool CoallocationScheduler::groupFinished(const sdpa::job_id_t& jid)
{
  lock_type lock(mtx_alloc_table_);
  allocation_table_t::iterator it=allocation_table_.find(jid);
  if(it!=allocation_table_.end())
    return it->second->allGroupTasksFinishedSuccessfully();
  else
    throw JobNotFoundException(jid);
}

}}
