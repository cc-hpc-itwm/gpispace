// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
        ( boost::function<void (const sdpa::worker_id_list_t&, const job_id_t&)> serve_job
        , boost::function<job_requirements_t (const sdpa::job_id_t&)> job_requirements
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
      std::set<worker_id_t> find_assignment_for_job
        ( worker_id_list_t available_workers
        , job_requirements_t requirements
        , boost::function<boost::optional<worker_id_t>
                            (job_requirements_t, worker_id_list_t)
                         > best_match
        )
      {
        std::size_t remaining_workers_needed (requirements.numWorkers());
        std::set<worker_id_t> workers;

        while (remaining_workers_needed --> 0)
        {
          const boost::optional<worker_id_t> matching_worker
            ( available_workers.empty() ? boost::none
            : requirements.empty() ? available_workers.front()
            : best_match (requirements, available_workers)
            );

          if (!matching_worker)
          {
            return std::set<worker_id_t>();
          }

          available_workers.erase
            ( std::find ( available_workers.begin(), available_workers.end()
                        , *matching_worker
                        )
            );

          workers.insert (*matching_worker);
        }

        return workers;
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

        const std::set<worker_id_t> matching_workers
          ( find_assignment_for_job
            ( listAvailWorkers
            , _job_requirements (jobId)
            , boost::bind
              (&WorkerManager::getBestMatchingWorker, &worker_manager(), _1, _2)
            )
          );

        if (!matching_workers.empty())
        {
          BOOST_FOREACH (worker_id_t const& worker, matching_workers)
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
              BOOST_FOREACH (worker_id_t const& worker, matching_workers)
              {
                worker_manager().findWorker (worker)->submit (jobId);
              }
              _serve_job (worker_id_list_t (matching_workers.begin(), matching_workers.end()), jobId);

              Reservation* pReservation (new Reservation (matching_workers));
              allocation_table_.insert (std::make_pair (jobId, pReservation));
            }
            catch (std::runtime_error const&)
            {
              BOOST_FOREACH (const worker_id_t& wid, matching_workers)
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

      BOOST_FOREACH (job_id_t id, jobs_to_schedule)
      {
        _jobs_to_schedule.push (id);
      }

      BOOST_FOREACH (const sdpa::job_id_t& id, nonmatching_jobs_queue)
      {
        _jobs_to_schedule.push (id);
      }
    }

    void CoallocationScheduler::releaseReservation (const sdpa::job_id_t& jobId)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);

      const allocation_table_t::const_iterator it
        (allocation_table_.find (jobId));

      if (it != allocation_table_.end())
      {
        delete it->second;
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
