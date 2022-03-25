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

#pragma once

#include <sdpa/daemon/Assignment.hpp>
#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
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

      virtual ~SingleAllocationScheduler() override = default;
      SingleAllocationScheduler (SingleAllocationScheduler const&) = delete;
      SingleAllocationScheduler (SingleAllocationScheduler&&) = delete;
      SingleAllocationScheduler& operator= (SingleAllocationScheduler const&) = delete;
      SingleAllocationScheduler& operator= (SingleAllocationScheduler&&) = delete;

      virtual void release_reservation (sdpa::job_id_t const&) override;
      virtual void assign_jobs_to_workers() override;
      virtual bool cancel_job
        ( job_id_t const&
        , bool
        , std::function<void (fhg::com::p2p::address_t const&)>
        ) override;
      virtual void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void (fhg::com::p2p::address_t const&)>
        ) override;
      virtual void reschedule_worker_jobs_and_maybe_remove_worker
        ( fhg::com::p2p::address_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
        , std::function<void (Job* job)>
        ) override;
      virtual bool reschedule_job_if_the_reservation_was_canceled
        (job_id_t const& job, status::code const) override;
      virtual void start_pending_jobs
        ( std::function<void ( WorkerSet const&
                             , Implementation const&
                             , job_id_t const&
                             , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
                             )
                       >
        ) override;
      virtual void submit_job (sdpa::job_id_t const&) override;
      virtual void acknowledge_job_sent_to_worker
        (job_id_t const&, fhg::com::p2p::address_t const&) override;
      virtual ::boost::optional<job_result_type>
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
