// Copyright (C) 2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/capability.hpp>
#include <gspc/scheduler/daemon/Implementation.hpp>
#include <gspc/scheduler/daemon/WorkerSet.hpp>
#include <gspc/scheduler/daemon/scheduler/Reservation.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/com/address.hpp>

#include <optional>

#include <functional>
#include <string>


  namespace gspc::scheduler::daemon
  {
    class Scheduler
    {
    public:
      virtual ~Scheduler() = default;

      Scheduler() = default;
      Scheduler (Scheduler const&) = delete;
      Scheduler& operator= (Scheduler const&) = delete;
      Scheduler (Scheduler&&) = delete;
      Scheduler& operator= (Scheduler&&) = delete;

      virtual std::optional<job_result_type>
        store_individual_result_and_get_final_if_group_finished
          ( gspc::com::p2p::address_t const&
          , job_id_t const&
          , terminal_state const&
          ) = 0;
      virtual void submit_job (gspc::scheduler::job_id_t const&) = 0;
      virtual void release_reservation (gspc::scheduler::job_id_t const&) = 0;
      virtual void assign_jobs_to_workers() = 0;
      virtual bool cancel_job
        ( job_id_t const&
        , bool
        , std::function<void (gspc::com::p2p::address_t const&)>
        ) = 0;
      virtual void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void (gspc::com::p2p::address_t const&)>
        ) = 0;
      virtual void reschedule_worker_jobs_and_maybe_remove_worker
        ( gspc::com::p2p::address_t const&
        , std::function<Job* (gspc::scheduler::job_id_t const&)>
        , std::function<void (gspc::com::p2p::address_t const& addr, job_id_t const&)>
        , std::function<void (Job* job)>
        ) = 0;
      virtual bool reschedule_job_if_the_reservation_was_canceled
        (job_id_t const& job, status::code) = 0;
      virtual void start_pending_jobs
        (std::function<void ( WorkerSet const&
                            , Implementation const&
                            , job_id_t const&
                            , std::function<gspc::com::p2p::address_t (worker_id_t const&)>
                            )
                      >
        ) = 0;
      virtual void acknowledge_job_sent_to_worker
        (job_id_t const&, gspc::com::p2p::address_t const&) = 0;
    };
  }
