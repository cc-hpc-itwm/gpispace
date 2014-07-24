// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

#include <functional>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
        ( std::function<void (const sdpa::worker_id_list_t&, const job_id_t&)> serve_job
        , std::function<job_requirements_t (const sdpa::job_id_t&)> job_requirements
        )
      : _serve_job (serve_job)
      , _job_requirements (job_requirements)
      , _worker_manager()
    {}

    const WorkerManager& CoallocationScheduler::worker_manager() const
    {
      return _worker_manager;
    }
    WorkerManager& CoallocationScheduler::worker_manager()
    {
      return _worker_manager;
    }

    bool CoallocationScheduler::delete_job (sdpa::job_id_t const& job)
    {
      return _jobs_to_schedule.erase (job);
    }

    void CoallocationScheduler::enqueueJob (const sdpa::job_id_t& jobId)
    {
      _jobs_to_schedule.push (jobId);
    }

    namespace
    {
      worker_id_set_t find_job_assignment_minimizing_memory_transfer_cost
        ( const mmap_match_deg_worker_id_t& mmap_matching_workers
        , size_t n_req_workers
        /*,  const we::type::activity_t& job_activity*/
        )
      {
        worker_id_set_t assigned_workers;

        // to do: pass as parameter the associated activity
        // and calculate the memory transfer costs w.r.t. the corresponding module call
        // and the matching multimap of workers (sorted in decreasing order of their matching
        // degrees, the multimap being implemented as a binary search tree.
        // Among all the workers matcbing the requirements,
        // choose n_req_workers s.t. the the total memory transfer cost
        // for the activity given as argument is minimized

        mmap_match_deg_worker_id_t::const_iterator it_last (mmap_matching_workers.begin());
        std::advance (it_last, n_req_workers);

        std::for_each ( mmap_matching_workers.begin()
                      , it_last
                      , [&assigned_workers] (const std::pair<int, worker_id_host_info_t>& pair_deg_worker_info)
                          {assigned_workers.insert (pair_deg_worker_info.second.worker_id());}
                      );

        return assigned_workers;
      }

      worker_id_set_t find_assignment_for_job
        ( const worker_id_list_t& available_workers
        , const job_requirements_t& requirements
        , std::function<mmap_match_deg_worker_id_t
                          ( const job_requirements_t&
                          , const worker_id_list_t&
                          )
                       > match_requirements
        )
      {
        mmap_match_deg_worker_id_t
          mmap_matching_workers (match_requirements (requirements, available_workers));

        if (mmap_matching_workers.size() >= requirements.numWorkers())
        {
          return find_job_assignment_minimizing_memory_transfer_cost ( mmap_matching_workers
                                                                     , requirements.numWorkers()
                                                                     );
        }

        return  worker_id_set_t();
      }
    }

    void CoallocationScheduler::assignJobsToWorkers()
    {
      sdpa::worker_id_list_t listAvailWorkers
        (worker_manager().getListWorkersNotReserved());

      std::list<job_id_t> jobs_to_schedule (_jobs_to_schedule.get_and_clear());

      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!listAvailWorkers.empty() && !jobs_to_schedule.empty())
      {
        sdpa::job_id_t jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const worker_id_set_t matching_workers
          ( find_assignment_for_job
            ( listAvailWorkers
            , _job_requirements (jobId)
            , std::bind
              ( &WorkerManager::getListMatchingWorkers
              , &worker_manager()
              , std::placeholders::_1
              , std::placeholders::_2
              )
            )
          );

        if (!matching_workers.empty())
        {
          for (worker_id_t const& worker : matching_workers)
          {
            listAvailWorkers.erase
              ( std::find
                (listAvailWorkers.begin(), listAvailWorkers.end(), worker)
              );
          }

          boost::mutex::scoped_lock const _ (mtx_alloc_table_);

          allocation_table_t::iterator it (allocation_table_.find (jobId));
          if (it != allocation_table_.end())
          {
            throw std::runtime_error ("already have reservation for job");
          }

          try
          {
            for (worker_id_t const& worker : matching_workers)
            {
              worker_manager().findWorker (worker)->submit (jobId);
            }
            _serve_job (worker_id_list_t (matching_workers.begin(), matching_workers.end()), jobId);

            Reservation* pReservation (new Reservation (matching_workers));
            allocation_table_.emplace (jobId, pReservation);
          }
          catch (std::runtime_error const&)
          {
            for (const worker_id_t& wid : matching_workers)
            {
              worker_manager().findWorker (wid)->deleteJob (jobId);
            }

            jobs_to_schedule.push_front (jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      for (job_id_t id : jobs_to_schedule)
      {
        _jobs_to_schedule.push (id);
      }

      for (const sdpa::job_id_t& id : nonmatching_jobs_queue)
      {
        _jobs_to_schedule.push (id);
      }
    }

    bool CoallocationScheduler::cancelNotTerminatedWorkerJobs ( std::function<void (worker_id_t const&)> func
                                                              , const sdpa::job_id_t& job_id)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      sdpa::worker_id_list_t list_not_terminated_workers;

      const allocation_table_t::const_iterator it
        (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        Reservation* ptr_reservation(it->second);
        list_not_terminated_workers = ptr_reservation->getListNotTerminatedWorkers();
      }

      for (worker_id_t worker_id : list_not_terminated_workers)
      {
        func (worker_id);
      }

      return !list_not_terminated_workers.empty();
    }

    void CoallocationScheduler::releaseReservation (const sdpa::job_id_t& job_id)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      const allocation_table_t::const_iterator it
       (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        Reservation* ptr_reservation(it->second);
        for (std::string worker : ptr_reservation->getWorkerList())
        {
          try {
              worker_manager().findWorker (worker)->deleteJob (job_id);
          }
          catch (const WorkerNotFoundException&)
          {
            // the worker might be gone in between
          }
        }

        delete ptr_reservation;
        allocation_table_.erase (it);
      }
    }

    void CoallocationScheduler::workerFinished
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::FINISHED);
      }
      else
      {
        throw std::runtime_error
          ("workerFinished: job missing in allocation table");
      }
    }

    void CoallocationScheduler::workerFailed
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::FAILED);
      }
      else
      {
        throw std::runtime_error
          ("workerFailed: job missing in allocation table");
      }
    }

    void CoallocationScheduler::workerCanceled
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::CANCELED);
      }
      else
      {
        throw std::runtime_error
          ("workerCanceled: job missing in allocation table");
      }
    }

    bool CoallocationScheduler::allPartialResultsCollected (const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        return it->second->allWorkersTerminated();
      }
      else
      {
        throw std::runtime_error
          ("allPartialResultsCollected: job missing in allocation table");
      }
    }

    bool CoallocationScheduler::groupFinished (const sdpa::job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        return it->second->allGroupTasksFinishedSuccessfully();
      }
      else
      {
        throw std::runtime_error
          ("groupFinished: job missing in allocation table");
      }
    }

    worker_id_list_t CoallocationScheduler::workers (job_id_t job_id) const
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::const_iterator it = allocation_table_.find (job_id);
      if (it != allocation_table_.end())
      {
        return it->second->getWorkerList();
      }
      else
      {
        throw std::runtime_error
          ("workers: job missing in allocation table");
      }
    }

    void CoallocationScheduler::locked_job_id_list::push (job_id_t item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      container_.push_back (item);
    }

    size_t CoallocationScheduler::locked_job_id_list::erase (const job_id_t& item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      size_t count (0);
      std::list<job_id_t>::iterator iter (container_.begin());
      while (iter != container_.end())
      {
        if (item == *iter)
        {
          iter = container_.erase(iter);
          ++count;
        }
        else
        {
          ++iter;
        }
      }
      return count;
    }

    std::list<job_id_t> CoallocationScheduler::locked_job_id_list::get_and_clear()
    {
      boost::mutex::scoped_lock const _ (mtx_);

      std::list<job_id_t> ret;
      std::swap (ret, container_);
      return ret;
    }
  }
}
