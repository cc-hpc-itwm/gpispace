// Copyright (C) 2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/daemon/Implementation.hpp>
#include <gspc/scheduler/daemon/Job.hpp>
#include <gspc/scheduler/daemon/WorkerManager.hpp>
#include <gspc/scheduler/daemon/WorkerSet.hpp>
#include <gspc/scheduler/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <gspc/scheduler/daemon/scheduler/Reservation.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/com/address.hpp>

#include <optional>

#include <algorithm>
#include <functional>
#include <mutex>
#include <random>
#include <unordered_set>


  namespace gspc::scheduler::daemon
  {
    class GreedyScheduler : public Scheduler
    {
    public:
      GreedyScheduler
        ( std::function
            < gspc::we::type::Requirements_and_preferences (gspc::scheduler::job_id_t const&)
            >
        , WorkerManager&
        );

      GreedyScheduler
        ( std::function
            < gspc::we::type::Requirements_and_preferences (gspc::scheduler::job_id_t const&)
            >
        , WorkerManager&
        , std::mt19937 const&
        );

      ~GreedyScheduler() override = default;
      GreedyScheduler (GreedyScheduler const&) = delete;
      GreedyScheduler (GreedyScheduler&&) = delete;
      GreedyScheduler& operator= (GreedyScheduler const&) = delete;
      GreedyScheduler& operator= (GreedyScheduler&&) = delete;

      void release_reservation (gspc::scheduler::job_id_t const&) override;
      void assign_jobs_to_workers() override;
      bool cancel_job
        ( job_id_t const&
        , bool
        , std::function<void (gspc::com::p2p::address_t const&)>
        ) override;
      void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void (gspc::com::p2p::address_t const&)>
        ) override;
      void reschedule_worker_jobs_and_maybe_remove_worker
        ( gspc::com::p2p::address_t const&
        , std::function<Job* (gspc::scheduler::job_id_t const&)>
        , std::function<void (gspc::com::p2p::address_t const& addr, job_id_t const&)>
        , std::function<void (Job* job)>
        ) override;
      bool reschedule_job_if_the_reservation_was_canceled
        (job_id_t const& job, status::code) override;
      void start_pending_jobs
        ( std::function<void ( WorkerSet const&
                             , Implementation const&
                             , job_id_t const&
                             , std::function<gspc::com::p2p::address_t (worker_id_t const&)>
                             )
                       >
        ) override;
      void submit_job (gspc::scheduler::job_id_t const&) override;
      void acknowledge_job_sent_to_worker
        (job_id_t const&, gspc::com::p2p::address_t const&) override;
      std::optional<job_result_type>
        store_individual_result_and_get_final_if_group_finished
          ( gspc::com::p2p::address_t const&
          , job_id_t const&
          , terminal_state const&
          ) override;

      CostAwareWithWorkStealingStrategy& strategy_TESTING_ONLY();

    private:
      CostAwareWithWorkStealingStrategy _strategy;
      std::mt19937 _random_engine;
    };
  }
