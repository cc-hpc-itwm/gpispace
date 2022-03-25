// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/scheduler/SingleAllocationScheduler.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/types.hpp>

#include <fhgcom/address.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <climits>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
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
    SingleAllocationScheduler::SingleAllocationScheduler
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
            requirements_and_preferences
        , WorkerManager& worker_manager
        )
      : _strategy (requirements_and_preferences, worker_manager)
    {}

    void SingleAllocationScheduler::assign_jobs_to_workers()
    {
      std::list<job_id_t> jobs_to_schedule
        (_strategy._jobs_to_schedule.get_and_clear());
      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t const jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const Requirements_and_preferences requirements_and_preferences
          (_strategy._requirements_and_preferences (jobId));

        if (requirements_and_preferences.numWorkers() != 1)
        {
          throw std::logic_error
            ( "More than one worker requested! Please use "
              "scheduling with coallocation if this was intended."
            );
        }

        std::lock_guard<std::mutex> const lock_alloc_table
          (_strategy.mtx_alloc_table_);
        std::lock_guard<std::mutex> const lock_worker_man
          (_strategy._worker_manager._mutex);

        auto const assignment
          (find_assignment (requirements_and_preferences, lock_worker_man));

        if (!assignment._workers.empty())
        {
          _strategy.assign_job
            ( jobId
            , assignment._workers
            , assignment._total_transfer_cost + requirements_and_preferences.computational_cost()
            , assignment._implementation
            , requirements_and_preferences.preferences()
            , jobs_to_schedule
            , lock_alloc_table
            , lock_worker_man
            );
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

    void SingleAllocationScheduler::start_pending_jobs
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

      for ( auto const& class_and_workers
          :  _strategy._worker_manager.classes_and_workers()
          )
      {
        if ( _strategy._worker_manager.num_pending_jobs (class_and_workers.first) == 0
           || _strategy._worker_manager.num_running_jobs (class_and_workers.first) == class_and_workers.second.size()
           )
        {
          continue;
        }

        for (auto const& worker_id : class_and_workers.second)
        {
          auto pending_job
            (_strategy._worker_manager.get_next_worker_pending_job_to_submit (worker_id));

          if (pending_job)
          {
            auto const implementation
              (_strategy.allocation_table_.at (*pending_job).get()->implementation());

            serve_job
              ( {worker_id}
              , implementation
              , *pending_job
              , [this] (worker_id_t const& worker)
                {
                  return _strategy._worker_manager.address_by_worker (worker).get()->second;
                }
              );
          }
        }
      }
    }

    void SingleAllocationScheduler::reschedule_worker_jobs_and_maybe_remove_worker
      ( fhg::com::p2p::address_t const& source
      , std::function<Job* (sdpa::job_id_t const&)> get_job
      , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
      , std::function<void (Job* job)> notify_job_failed
      )
    {
      _strategy.reschedule_worker_jobs_and_maybe_remove_worker
        (source, get_job, notify_job_failed);
    }

    void SingleAllocationScheduler::release_reservation
      (sdpa::job_id_t const& job_id)
    {
      std::lock_guard<std::mutex> const lock_alloc_table (_strategy.mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_strategy._worker_manager._mutex);

      _strategy.release_reservation (job_id, lock_alloc_table, lock_worker_man);
    }

    bool SingleAllocationScheduler::reschedule_job_if_the_reservation_was_canceled
      (job_id_t const&, status::code const)
    {
      return false;
    }

    bool SingleAllocationScheduler::cancel_job
      ( job_id_t const& job_id
      , bool
      , std::function<void (fhg::com::p2p::address_t const&)> send_cancel
      )
    {
      return _strategy.cancel_job (job_id, send_cancel);
    }

    void SingleAllocationScheduler::notify_submitted_or_acknowledged_workers
      ( job_id_t const& job_id
      , std::function<void ( fhg::com::p2p::address_t const&)> notify_workers
      )
    {
      _strategy.notify_submitted_or_acknowledged_workers (job_id, notify_workers);
    }

    Assignment SingleAllocationScheduler::find_assignment
      ( Requirements_and_preferences const& requirements_and_preferences
      , std::lock_guard<std::mutex> const&
      ) const
    {
      size_t const num_required_workers
        (requirements_and_preferences.numWorkers());

      if (num_required_workers != 1)
      {
        throw std::logic_error
          ( "More than one worker requested! Please use "
            "scheduling with coallocation if this was intended."
          );
      }

      if (_strategy._worker_manager.number_of_workers() < num_required_workers)
      {
        return {{}, ::boost::none, 0.0, {}};
      }

      std::unique_ptr<struct CostsAndMatchingWorkerInfo> best_worker_matching_info (nullptr);
      Compare compare;

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

          CostsAndMatchingWorkerInfo worker_matching_info
            { total_cost
            , -1.0 * matching_degree_and_implementation.first.get()
            , shared_memory_size
            , last_time_idle
            , worker_id
            , matching_degree_and_implementation.second
            , transfer_cost
            , worker_class.first
            };

          if ( !best_worker_matching_info
             || compare (worker_matching_info, *best_worker_matching_info)
             )
          {
            best_worker_matching_info = std::make_unique<CostsAndMatchingWorkerInfo>
                                          (worker_matching_info);
          }
        }
      }

      if (best_worker_matching_info)
      {
        return { {best_worker_matching_info->_worker_id}
               , best_worker_matching_info->_implementation
               , best_worker_matching_info->_transfer_cost
               , {}
               };
      }

      return {{}, ::boost::none, 0.0, {}};
    }

    void SingleAllocationScheduler::submit_job (sdpa::job_id_t const& job)
    {
      _strategy.submit_job (job);
    }

    void SingleAllocationScheduler::acknowledge_job_sent_to_worker
      (job_id_t const& job, fhg::com::p2p::address_t const& address)
    {
      _strategy.acknowledge_job_sent_to_worker (job, address);
    }

    ::boost::optional<job_result_type> SingleAllocationScheduler::
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
      SingleAllocationScheduler::strategy_TESTING_ONLY()
    {
      return _strategy;
    }
  }
}
