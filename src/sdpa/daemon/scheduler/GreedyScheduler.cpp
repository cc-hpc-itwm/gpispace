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
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/scheduler/GreedyScheduler.hpp>
#include <sdpa/types.hpp>

#include <fhgcom/address.hpp>

#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>

#include <algorithm>
#include <climits>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace
{
  template<typename Iterator, typename RandomGenerator>
  Iterator choose_randomly (Iterator start, Iterator end, RandomGenerator& g)
  {
    std::uniform_int_distribution<> dis (0, std::distance (start, end) - 1);
    std::advance (start, dis (g));
    return start;
  }
}

namespace sdpa
{
  namespace daemon
  {
    GreedyScheduler::GreedyScheduler
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
            requirements_and_preferences
        , WorkerManager& worker_manager
        , std::mt19937 const& random_engine
        )
      : _strategy (requirements_and_preferences, worker_manager)
      , _random_engine (random_engine)
    {}

    GreedyScheduler::GreedyScheduler
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
            requirements_and_preferences
        , WorkerManager& worker_manager
        )
      : GreedyScheduler
          ( requirements_and_preferences
          , worker_manager
          , std::mt19937 (std::random_device()())
          )
    {}

    void GreedyScheduler::assign_jobs_to_workers()
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

        ::boost::optional<worker_id_t> assigned_worker;

        for ( auto const& worker_class
            : _strategy._worker_manager.classes_and_workers()
            )
        {
          if (worker_class.second.empty())
          {
            continue;
          }

          auto const matching_degree_and_implementation
            ( _strategy.match_requirements_and_preferences
                (requirements_and_preferences, worker_class.first)
            );

          if (matching_degree_and_implementation.first)
          {
            auto const worker { *choose_randomly
                                  ( worker_class.second.begin()
                                  , worker_class.second.end()
                                  , _random_engine
                                  )
                              };

            if ( _strategy.assign_job
                   ( jobId
                   , {worker}
                   , 0.0
                   , matching_degree_and_implementation.second
                   , requirements_and_preferences.preferences()
                   , jobs_to_schedule
                   , lock_alloc_table
                   , lock_worker_man
                   )
               )
            {
              assigned_worker = worker;
              break;
            }
          }
        }

        if (!assigned_worker)
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      _strategy._jobs_to_schedule.push (jobs_to_schedule);
      _strategy._jobs_to_schedule.push (nonmatching_jobs_queue);

      _strategy.steal_work (NotUsingCosts{});
    }

    void GreedyScheduler::start_pending_jobs
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

    void GreedyScheduler::reschedule_worker_jobs_and_maybe_remove_worker
      ( fhg::com::p2p::address_t const& source
      , std::function<Job* (sdpa::job_id_t const&)> get_job
      , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
      , std::function<void (Job* job)> notify_job_failed
      )
    {
      _strategy.reschedule_worker_jobs_and_maybe_remove_worker
        (source, get_job, notify_job_failed);
    }

    void GreedyScheduler::release_reservation
      (sdpa::job_id_t const& job_id)
    {
      std::lock_guard<std::mutex> const lock_alloc_table (_strategy.mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_strategy._worker_manager._mutex);

      _strategy.release_reservation (job_id, lock_alloc_table, lock_worker_man);
    }

    bool GreedyScheduler::reschedule_job_if_the_reservation_was_canceled
      (job_id_t const&, status::code const)
    {
      return false;
    }

    bool GreedyScheduler::cancel_job
      ( job_id_t const& job_id
      , bool
      , std::function<void (fhg::com::p2p::address_t const&)> send_cancel
      )
    {
      return _strategy.cancel_job (job_id, send_cancel);
    }

    void GreedyScheduler::notify_submitted_or_acknowledged_workers
      ( job_id_t const& job_id
      , std::function<void ( fhg::com::p2p::address_t const&)> notify_workers
      )
    {
      _strategy.notify_submitted_or_acknowledged_workers (job_id, notify_workers);
    }

    void GreedyScheduler::submit_job (sdpa::job_id_t const& job)
    {
      _strategy.submit_job (job);
    }

    void GreedyScheduler::acknowledge_job_sent_to_worker
      (job_id_t const& job, fhg::com::p2p::address_t const& address)
    {
      _strategy.acknowledge_job_sent_to_worker (job, address);
    }

    ::boost::optional<job_result_type> GreedyScheduler::
      store_individual_result_and_get_final_if_group_finished
        ( fhg::com::p2p::address_t const& address
        , job_id_t const& job
        , terminal_state const& state
        )
    {
      return _strategy.store_individual_result_and_get_final_if_group_finished
        (address, job, state);
    }

    CostAwareWithWorkStealingStrategy& GreedyScheduler::strategy_TESTING_ONLY()
    {
      return _strategy;
    }
  }
}
