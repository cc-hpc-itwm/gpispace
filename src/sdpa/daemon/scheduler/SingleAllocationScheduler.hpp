// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/daemon/Assignment.hpp>
#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/types.hpp>

#include <fhgcom/address.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <functional>
#include <mutex>
#include <unordered_set>

namespace sdpa
{
  namespace daemon
  {
    class SingleAllocationScheduler : public Scheduler
    {
    public:
      SingleAllocationScheduler
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
        , WorkerManager&
        );

      ~SingleAllocationScheduler() override = default;
      SingleAllocationScheduler (SingleAllocationScheduler const&) = delete;
      SingleAllocationScheduler (SingleAllocationScheduler&&) = delete;
      SingleAllocationScheduler& operator= (SingleAllocationScheduler const&) = delete;
      SingleAllocationScheduler& operator= (SingleAllocationScheduler&&) = delete;

      void release_reservation (sdpa::job_id_t const&) override;
      void assign_jobs_to_workers() override;
      bool cancel_job
        ( job_id_t const&
        , bool
        , std::function<void (fhg::com::p2p::address_t const&)>
        ) override;
      void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void (fhg::com::p2p::address_t const&)>
        ) override;
      void reschedule_worker_jobs_and_maybe_remove_worker
        ( fhg::com::p2p::address_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
        , std::function<void (Job* job)>
        ) override;
      bool reschedule_job_if_the_reservation_was_canceled
        (job_id_t const& job, status::code) override;
      void start_pending_jobs
        ( std::function<void ( WorkerSet const&
                             , Implementation const&
                             , job_id_t const&
                             , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
                             )
                       >
        ) override;
      void submit_job (sdpa::job_id_t const&) override;
      void acknowledge_job_sent_to_worker
        (job_id_t const&, fhg::com::p2p::address_t const&) override;
      ::boost::optional<job_result_type>
        store_individual_result_and_get_final_if_group_finished
          ( fhg::com::p2p::address_t const&
          , job_id_t const&
          , terminal_state const&
          ) override;

      CostAwareWithWorkStealingStrategy& strategy_TESTING_ONLY();

    private:
      CostAwareWithWorkStealingStrategy _strategy;

      Assignment find_assignment
          ( Requirements_and_preferences const&
          , std::lock_guard<std::mutex> const&
          ) const;
    };
  }
}
