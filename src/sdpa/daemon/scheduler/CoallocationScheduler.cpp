// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/daemon/Assignment.hpp>
#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/types.hpp>

#include <fhgcom/address.hpp>

#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>

#include <algorithm>
#include <climits>
#include <functional>
#include <iterator>
#include <list>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace sdpa
{
  namespace daemon
  {
    namespace
    {
      using base_priority_queue_t =
        std::priority_queue< CostsAndMatchingWorkerInfo
                           , std::vector<CostsAndMatchingWorkerInfo>
                           , Compare
                           >;

      class bounded_priority_queue_t : private base_priority_queue_t
      {
      public:
        explicit bounded_priority_queue_t (std::size_t capacity)
          : capacity_ (capacity)
        {}

        template<typename... Args>
          void emplace (Args&&... args)
        {
          if (size() < capacity_)
          {
            base_priority_queue_t::emplace (std::forward<Args> (args)...);
            return;
          }

          CostsAndMatchingWorkerInfo const next_tuple (std::forward<Args> (args)...);

          if (comp (next_tuple, top()))
          {
            pop();
            base_priority_queue_t::emplace (std::move (next_tuple));
          }
        }

        Assignment assigned_workers_and_implementation() const
        {
          WorkerSet workers;
          auto implementation (c.front()._implementation);
          double total_transfer_cost (0.0);

          std::transform ( c.begin()
                         , c.end()
                         , std::inserter (workers, workers.begin())
                         , [&total_transfer_cost]
                           (CostsAndMatchingWorkerInfo const& cost_and_matching_info)
                           {
                             total_transfer_cost += cost_and_matching_info._transfer_cost;
                             return cost_and_matching_info._worker_id;
                           }
                         );


          return {workers, implementation, total_transfer_cost, c.front()._worker_class};
        }

        std::size_t size() const { return base_priority_queue_t::size(); }
      private:
        size_t capacity_;
      };
    }

    CoallocationScheduler::CoallocationScheduler
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
           requirements_and_preferences
        , WorkerManager& worker_manager
        )
      : _strategy (requirements_and_preferences, worker_manager)
    {}

    void CoallocationScheduler::assign_jobs_to_workers()
    {
      std::list<job_id_t> jobs_to_schedule (_strategy._jobs_to_schedule.get_and_clear());
      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t const jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const Requirements_and_preferences requirements_and_preferences
          (_strategy._requirements_and_preferences (jobId));

        if ( !requirements_and_preferences.preferences().empty()
           && requirements_and_preferences.numWorkers() > 1
           )
        {
          throw std::runtime_error
            ("Coallocation with preferences is forbidden!");
        }

        std::lock_guard<std::mutex> const lock_alloc_table
          (_strategy.mtx_alloc_table_);
        std::lock_guard<std::mutex> const lock_worker_man
          (_strategy._worker_manager._mutex);

        auto const assignment
          (find_assignment (requirements_and_preferences, lock_worker_man));

        if (!assignment._workers.empty())
        {
          if ( _strategy.assign_job
                ( jobId
                , assignment._workers
                , assignment._total_transfer_cost + requirements_and_preferences.computational_cost()
                , assignment._implementation
                , requirements_and_preferences.preferences()
                , jobs_to_schedule
                , lock_alloc_table
                , lock_worker_man
                )
             )
          {
            _strategy._pending_jobs.add (assignment._worker_class, jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      _strategy._jobs_to_schedule.push (jobs_to_schedule);
      _strategy._jobs_to_schedule.push (nonmatching_jobs_queue);

      _strategy.steal_work (UsingCosts{});
    }

    void CoallocationScheduler::start_pending_jobs
      ( std::function<void ( WorkerSet const&
                           , Implementation const& implementation
                           , job_id_t const&
                           , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
                           )
                     > serve_job
      )
    {
      std::lock_guard<std::mutex> const _ (_strategy.mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_strategy._worker_manager._mutex);

      for ( auto& class_and_pending_jobs
          : _strategy._pending_jobs()
          )
      {
        for ( auto it (class_and_pending_jobs.second.begin())
            ; it != class_and_pending_jobs.second.end()
            && _strategy._worker_manager.num_free_workers (class_and_pending_jobs.first) > 0
            ;
            )
        {
          auto const job_id (*it);

          auto const job_reservation (_strategy.allocation_table_.at (job_id).get());
          auto const workers (job_reservation->workers());
          auto const implementation (job_reservation->implementation());

          if (_strategy._worker_manager.all_free (workers))
          {
            for (auto const& worker: workers)
            {
              _strategy._worker_manager.submit_job_to_worker (job_id, worker);
            }

            serve_job
              ( workers
              , implementation
              , job_id
              , [this] (worker_id_t const& worker)
                {
                  return _strategy._worker_manager.address_by_worker (worker).get()->second;
                }
              );

            it = class_and_pending_jobs.second.erase (it);
          }
          else
          {
            it++;
          }
        }
      }
    }

    void CoallocationScheduler::reschedule_worker_jobs_and_maybe_remove_worker
      ( fhg::com::p2p::address_t const& source
      , std::function<Job* (sdpa::job_id_t const&)> get_job
      , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)> cancel_worker_job
      , std::function<void (Job* job)> notify_job_failed
      )
    {
      _strategy.reschedule_worker_jobs_and_maybe_remove_worker
        (source, get_job, cancel_worker_job, notify_job_failed);
    }

    void CoallocationScheduler::release_reservation (sdpa::job_id_t const& job_id)
    {
      std::lock_guard<std::mutex> const lock_alloc_table (_strategy.mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_strategy._worker_manager._mutex);

      _strategy.release_reservation (job_id, lock_alloc_table, lock_worker_man);
    }

    bool CoallocationScheduler::reschedule_job_if_the_reservation_was_canceled
      (job_id_t const& job, status::code const status)
    {
      std::lock_guard<std::mutex> const lock_alloc_table (_strategy.mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_strategy._worker_manager._mutex);

      if ( !_strategy.allocation_table_.at (job)->is_canceled()
        || (status == sdpa::status::CANCELING)
         )
      {
        return false;
      }

      _strategy.release_reservation (job, lock_alloc_table, lock_worker_man);

      return true;
    }

    bool CoallocationScheduler::cancel_job
      ( job_id_t const& job_id
      , bool cancel_already_requested
      , std::function<void (fhg::com::p2p::address_t const&)> send_cancel
      )
    {
      return _strategy.cancel_job (job_id, cancel_already_requested, send_cancel);
    }

    void CoallocationScheduler::notify_submitted_or_acknowledged_workers
      ( job_id_t const& job_id
      , std::function<void ( fhg::com::p2p::address_t const&)> notify_workers
      )
    {
      _strategy.notify_submitted_or_acknowledged_workers (job_id, notify_workers);
    }

    Assignment CoallocationScheduler::find_assignment
      ( Requirements_and_preferences const& requirements_and_preferences
      , std::lock_guard<std::mutex> const&
      ) const
    {
      size_t const num_required_workers
        (requirements_and_preferences.numWorkers());

      if (_strategy._worker_manager.number_of_workers() < num_required_workers)
      {
        return {{}, ::boost::none, 0.0, {}};
      }

      bounded_priority_queue_t bpq (num_required_workers);

      for (auto const& worker_class : _strategy._worker_manager.classes_and_workers())
      {
        auto const matching_degree_and_implementation
          ( _strategy.match_requirements_and_preferences
              (requirements_and_preferences, worker_class.first)
          );

        if (!matching_degree_and_implementation.first)
        {
          continue;
        }

        for (auto& worker_id : worker_class.second)
        {
          double cost_assigned_jobs;
          unsigned long shared_memory_size;
          double last_time_idle;
          double transfer_cost;

          std::tie (cost_assigned_jobs, shared_memory_size, last_time_idle, transfer_cost)
            = _strategy._worker_manager.costs_memory_size_and_last_idle_time
                (worker_id, requirements_and_preferences);

          if ( requirements_and_preferences.shared_memory_amount_required()
             > shared_memory_size
             )
            { continue; }

          double const total_cost
            ( cost_assigned_jobs
            + requirements_and_preferences.computational_cost()
            + transfer_cost
            );

          bpq.emplace ( total_cost
                      , -1.0 * matching_degree_and_implementation.first.get()
                      , shared_memory_size
                      , last_time_idle
                      , worker_id
                      , matching_degree_and_implementation.second
                      , transfer_cost
                      , worker_class.first
                      );
        }
      }

      if (bpq.size() == num_required_workers)
      {
        return bpq.assigned_workers_and_implementation();
      }

      return {{}, ::boost::none, 0.0, {}};
    }

    void CoallocationScheduler::submit_job (sdpa::job_id_t const& job)
    {
      _strategy.submit_job (job);
    }

    void CoallocationScheduler::acknowledge_job_sent_to_worker
      (job_id_t const& job, fhg::com::p2p::address_t const& address)
    {
      _strategy.acknowledge_job_sent_to_worker (job, address);
    }

    ::boost::optional<job_result_type> CoallocationScheduler::
      store_individual_result_and_get_final_if_group_finished
        ( fhg::com::p2p::address_t const& address
        , job_id_t const& job
        , terminal_state const& state
        )
    {
      return _strategy.store_individual_result_and_get_final_if_group_finished
        (address, job, state);
    }

    CostAwareWithWorkStealingStrategy&
      CoallocationScheduler::strategy_TESTING_ONLY()
    {
      return _strategy;
    }
  }
}
