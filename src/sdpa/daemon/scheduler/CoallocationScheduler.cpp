#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

#include <boost/range/algorithm.hpp>

#include <climits>
#include <functional>
#include <queue>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
      ( std::function<job_requirements_t (const sdpa::job_id_t&)> job_requirements
      , WorkerManager& worker_manager
      )
      : _job_requirements (job_requirements)
      , _worker_manager (worker_manager)
    {}

    bool CoallocationScheduler::delete_job (sdpa::job_id_t const& job)
    {
      return _jobs_to_schedule.erase (job);
    }

    void CoallocationScheduler::enqueueJob (const sdpa::job_id_t& jobId)
    {
      _jobs_to_schedule.push (jobId);
    }

    double CoallocationScheduler::compute_reservation_cost
      ( const job_id_t& job_id
      , const std::set<worker_id_t>& workers
      , const double computational_cost
      ) const
    {
      return std::accumulate
        ( workers.begin()
        , workers.end()
        , 0.0
        , [this, &job_id] (const double total, const sdpa::worker_id_t wid)
          {
            return total
              + _job_requirements (job_id).transfer_cost()
                  (*_worker_manager.vmem_rank (wid));
          }
        ) + computational_cost;
    }

    void CoallocationScheduler::assignJobsToWorkers()
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      if (_worker_manager.all_workers_busy_and_have_pending_jobs())
      {
        return;
      }

      std::list<job_id_t> jobs_to_schedule (_jobs_to_schedule.get_and_clear());
      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t const jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const job_requirements_t requirements (_job_requirements (jobId));
        const std::set<worker_id_t> matching_workers
          (_worker_manager.find_assignment
            ( requirements
            , [this] (const job_id_t& job_id) -> double
              {
                return allocation_table_.at (job_id)->cost();
              }
            )
          );

        if (!matching_workers.empty())
        {
          if (allocation_table_.find (jobId) != allocation_table_.end())
          {
            throw std::runtime_error ("already have reservation for job " + jobId);
          }

          try
          {
            for (worker_id_t const& worker : matching_workers)
            {
              _worker_manager.assign_job_to_worker (jobId, worker);
            }

            Reservation* const pReservation
              (new Reservation ( matching_workers
                               , compute_reservation_cost ( jobId
                                                          , matching_workers
                                                          , requirements.computational_cost()
                                                          )
                               )
              );

            allocation_table_.emplace (jobId, pReservation);
            _pending_jobs.emplace (jobId);
          }
          catch (std::out_of_range const&)
          {
            for (const worker_id_t& wid : matching_workers)
            {
              _worker_manager.delete_job_from_worker (jobId, wid);
            }

            jobs_to_schedule.push_front (jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      _jobs_to_schedule.push (jobs_to_schedule);
      _jobs_to_schedule.push (nonmatching_jobs_queue);
    }

    void CoallocationScheduler::steal_work()
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      _worker_manager.steal_work<Reservation>
        ( [this] (job_id_t const& job)
          {
            return allocation_table_.at (job);
          }
        );
    }

    CoallocationScheduler::assignment_t
      CoallocationScheduler::get_current_assignment_TESTING_ONLY() const
    {
      assignment_t assignment;
      std::transform ( allocation_table_.begin()
                     , allocation_table_.end()
                     , std::inserter (assignment, assignment.end())
                     , [](allocation_table_t::value_type const &p)
                       {
                         return std::make_pair (p.first, p.second->workers());
                       }
                     );

      return assignment;
    }

    void CoallocationScheduler::reschedule_worker_jobs
       ( worker_id_t const& worker
       , std::function<Job* (sdpa::job_id_t const&)> get_job
       , std::function<void (sdpa::worker_id_t const&, job_id_t const&)> cancel_worker_job
       , bool backlog_full
       )
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);

      for ( job_id_t const& job_id
          : _worker_manager.delete_or_cancel_worker_jobs<Reservation>
              ( worker
              , get_job
              , [this] (job_id_t const& jobId)
                {
                  return allocation_table_.at (jobId);
                }
              , cancel_worker_job
              )
          )
      {
        releaseReservation (job_id);
        enqueueJob (job_id);
      }

      if (backlog_full)
      {
        _worker_manager.set_worker_backlog_full (worker, true);
      }
      else
      {
        _worker_manager.deleteWorker (worker);
      }
    }

    std::set<job_id_t> CoallocationScheduler::start_pending_jobs
      (std::function<void (std::set<worker_id_t> const&, const job_id_t&)> serve_job)
    {
      std::set<job_id_t> jobs_started;
      std::unordered_set<job_id_t> remaining_jobs;
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      for (const job_id_t& job_id: _pending_jobs)
      {
        std::set<worker_id_t> const& workers (allocation_table_.at (job_id)->workers());
        if (_worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
             (job_id, workers, serve_job)
           )
        {
          jobs_started.insert (job_id);
        }
        else
        {
          remaining_jobs.emplace (job_id);
        }
      }

      std::swap (_pending_jobs, remaining_jobs);

      return jobs_started;
    }

    void CoallocationScheduler::releaseReservation (const sdpa::job_id_t& job_id)
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      const allocation_table_t::const_iterator it
       (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        Reservation const* const ptr_reservation(it->second);
        for (std::string const& worker : ptr_reservation->workers())
        {
          try
          {
            _worker_manager.delete_job_from_worker (job_id, worker);
          }
          catch (...)
          {
            //! \note can be ignored: was deleted using deleteWorker()
            //! which correctly clears queues already, and
            //! delete_job_from_worker does nothing else.
          }
        }

        delete ptr_reservation;
        _pending_jobs.erase (it->first);
        allocation_table_.erase (it);
      }
      //! \todo why can we ignore this?
    }

    bool CoallocationScheduler::reservation_canceled (job_id_t const& job) const
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      return allocation_table_.at (job)->is_canceled();
    }

    void CoallocationScheduler::store_result ( worker_id_t const& worker_id
                                             , job_id_t const& job_id
                                             , terminal_state result
                                             )
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error ("store_result: unknown job");
      }

      it->second->store_result (worker_id, result);
    }

    boost::optional<job_result_type>
      CoallocationScheduler::get_aggregated_results_if_all_terminated (job_id_t const& job_id)
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error
          ("get_aggregated_results_if_all_terminated: unknown job");
      }

      return it->second->get_aggregated_results_if_all_terminated();
    }

    void CoallocationScheduler::locked_job_id_list::push (job_id_t const& item)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      container_.emplace_back (item);
    }

    template <typename Range>
    void CoallocationScheduler::locked_job_id_list::push (Range const& range)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      container_.insert (container_.end(), std::begin (range), std::end (range));
    }

    size_t CoallocationScheduler::locked_job_id_list::erase (const job_id_t& item)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
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
      std::lock_guard<std::mutex> const _ (mtx_);

      std::list<job_id_t> ret;
      std::swap (ret, container_);
      return ret;
    }
  }
}
