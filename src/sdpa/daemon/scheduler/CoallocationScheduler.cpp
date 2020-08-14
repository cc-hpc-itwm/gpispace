#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <boost/range/algorithm.hpp>

#include <climits>
#include <functional>
#include <queue>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
      ( std::function<Requirements_and_preferences (const sdpa::job_id_t&)> requirements_and_preferences
      , WorkerManager& worker_manager
      )
      : _requirements_and_preferences (requirements_and_preferences)
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

    void CoallocationScheduler::delete_pending_job (sdpa::job_id_t const& job)
    {
      _pending_jobs.erase (job);
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
        , [this, &job_id] ( const double total
                          , worker_id_t const& worker
                          )
          {
            return total
              + _requirements_and_preferences (job_id).transfer_cost()
                  (_worker_manager.host_INDICATES_A_RACE (worker));
          }
        ) + computational_cost;
    }

    void CoallocationScheduler::assignJobsToWorkers()
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);

      std::list<job_id_t> jobs_to_schedule (_jobs_to_schedule.get_and_clear());
      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t const jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const Requirements_and_preferences requirements_and_preferences
          (_requirements_and_preferences (jobId));

        if ( !requirements_and_preferences.preferences().empty()
           && requirements_and_preferences.numWorkers() > 1
           )
        {
          throw std::runtime_error
            ("Coallocation with preferences is forbidden!");
        }

        const Workers_and_implementation
          matching_workers_and_implementation
            (_worker_manager.find_assignment (requirements_and_preferences)
            );

        if (!matching_workers_and_implementation.first.empty())
        {
          if (allocation_table_.find (jobId) != allocation_table_.end())
          {
            throw std::runtime_error ("already have reservation for job " + jobId);
          }

          double cost
            (compute_reservation_cost
              ( jobId
              , matching_workers_and_implementation.first
              , requirements_and_preferences.computational_cost()
              )
            );

          try
          {
            for ( auto const& worker
                : matching_workers_and_implementation.first
                )
            {
              _worker_manager.assign_job_to_worker
                ( jobId
                , worker
                , cost
                , requirements_and_preferences.preferences()
                );
            }

            allocation_table_.emplace
              ( jobId
              , fhg::util::cxx14::make_unique<scheduler::Reservation>
                  ( matching_workers_and_implementation.first
                  , matching_workers_and_implementation.second
                  , requirements_and_preferences.preferences()
                  , cost
                  )
              );

            _pending_jobs.emplace (jobId);
          }
          catch (std::out_of_range const&)
          {
            for ( auto const&  worker
                : matching_workers_and_implementation.first
                )
            {
              _worker_manager.delete_job_from_worker (jobId, worker, cost);
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
      _worker_manager.steal_work
        ( [this] (job_id_t const& job)
          {
            return allocation_table_.at (job).get();
          }
        );
    }

    void CoallocationScheduler::reschedule_worker_jobs_and_maybe_remove_worker
       ( worker_id_t const& worker
       , std::function<Job* (sdpa::job_id_t const&)> get_job
       , std::function<void (sdpa::worker_id_t const&, job_id_t const&)> cancel_worker_job
       , bool backlog_full
       )
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);

      for ( job_id_t const& job_id
          : _worker_manager.delete_or_cancel_worker_jobs
              ( worker
              , get_job
              , [this] (job_id_t const& jobId)
                {
                  return allocation_table_.at (jobId).get();
                }
              , cancel_worker_job
              )
          )
      {
        releaseReservation (job_id);
        delete_pending_job (job_id);
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
      ( std::function<void ( WorkerSet const&
                           , Implementation const& implementation
                           , const job_id_t&
                           )
                     > serve_job
      )
    {
      std::set<job_id_t> jobs_started;
      bool started (false);

      long num_free_workers_left
        (_worker_manager.num_free_workers());

      for ( auto it (_pending_jobs.begin())
          ; it != _pending_jobs.end()
          ;
          )
      {
        auto const job_id (*it);

        std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
        auto const assigned_workers (allocation_table_.at (job_id)->workers());
        std::tie (started, num_free_workers_left) =
          _worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
            ( job_id
            , allocation_table_.at (job_id)->workers()
            , allocation_table_.at (job_id)->implementation()
            , serve_job
            );

        if (started)
        {
          jobs_started.insert (job_id);
          it = _pending_jobs.erase (it);
          if (!_worker_manager.resource_manager().has_free_resources())
            { break; }
        }
        else
        {
          it++;
        }
      }

      return jobs_started;
    }

    void CoallocationScheduler::releaseReservation (const sdpa::job_id_t& job_id)
    {
      std::lock_guard<std::recursive_mutex> const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        for (std::string const& worker : it->second->workers())
        {
          try
          {
            _worker_manager.delete_job_from_worker
              (job_id, worker, it->second->cost());
          }
          catch (...)
          {
            //! \note can be ignored: was deleted using deleteWorker()
            //! which correctly clears queues already, and
            //! delete_job_from_worker does nothing else.
          }
        }

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
