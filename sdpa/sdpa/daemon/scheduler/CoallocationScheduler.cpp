// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler (GenericDaemon* pCommHandler)
      : ptr_comm_handler_ ( pCommHandler
                          ? pCommHandler
                          : throw std::runtime_error
                            ("CoallocationScheduler ctor with NULL ptr_comm_handler")
                          )
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

      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!listAvailWorkers.empty())
      {
        const boost::optional<job_id_t> job_id (_jobs_to_schedule.pop());
        if (!job_id)
        {
          break;
        }
        sdpa::job_id_t jobId (*job_id);

        const job_requirements_t job_reqs
          (ptr_comm_handler_->getJobRequirements (jobId));

        const std::set<worker_id_t> matching_workers
          ( find_assignment_for_job
            ( listAvailWorkers
            , job_reqs
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
          {
            BOOST_FOREACH (worker_id_t const& worker, matching_workers)
            {
              worker_manager().findWorker (worker)->reserve();
            }

            allocation_table_t::iterator it (allocation_table_.find (jobId));
            if (it == allocation_table_.end())
            {
              Reservation* pReservation (new Reservation (job_reqs.numWorkers()));
              allocation_table_.insert (std::make_pair (jobId, pReservation));
            }

            BOOST_FOREACH (worker_id_t const& worker, matching_workers)
            {
              allocation_table_[jobId]->addWorker (worker);
            }
          }

          Reservation* pReservation (allocation_table_[jobId]);

            sdpa::worker_id_list_t list_reserved_workers =
              pReservation->getWorkerList();

            try
            {
              BOOST_FOREACH (const worker_id_t& wid, list_reserved_workers)
              {
                worker_manager().findWorker (wid)->submit (jobId);
              }
              ptr_comm_handler_->serveJob (list_reserved_workers, jobId);
            }
            catch (std::runtime_error const&)
            {
              BOOST_FOREACH (const worker_id_t& wid, list_reserved_workers)
              {
                worker_manager().findWorker (wid)->deleteJob (jobId);
                worker_manager().findWorker (wid)->free();
                pReservation->delWorker (wid);
              }

              _jobs_to_schedule.push_front (jobId);
            }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
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
        BOOST_FOREACH ( sdpa::worker_id_t const& workerId
                      , it->second->getWorkerList()
                      )
        {
          worker_manager().findWorker (workerId)->free();
        }

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
        throw JobNotFoundException();
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
        throw JobNotFoundException();
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
        throw JobNotFoundException();
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
        throw JobNotFoundException();
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
        throw JobNotFoundException();
      }
    }

    boost::optional<job_id_t> CoallocationScheduler::locked_job_id_list::pop()
    {
      boost::mutex::scoped_lock const _ (mtx_);
      if (container_.empty())
      {
        return boost::none;
      }

      job_id_t item = container_.front();
      container_.pop_front();
      return item;
    }

    void CoallocationScheduler::locked_job_id_list::push (job_id_t item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      container_.push_back (item);
    }

    void CoallocationScheduler::locked_job_id_list::push_front (job_id_t item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      container_.push_front (item);
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
  }
}
