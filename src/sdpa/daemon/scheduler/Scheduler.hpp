// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/capability.hpp>
#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/WorkerSet.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/types.hpp>

#include <fhgcom/address.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <string>

namespace sdpa
{
  namespace daemon
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

      virtual ::boost::optional<job_result_type>
        store_individual_result_and_get_final_if_group_finished
          ( fhg::com::p2p::address_t const&
          , job_id_t const&
          , terminal_state const&
          ) = 0;
      virtual void submit_job (sdpa::job_id_t const&) = 0;
      virtual void release_reservation (sdpa::job_id_t const&) = 0;
      virtual void assign_jobs_to_workers() = 0;
      virtual bool cancel_job
        ( job_id_t const&
        , bool
        , std::function<void (fhg::com::p2p::address_t const&)>
        ) = 0;
      virtual void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void (fhg::com::p2p::address_t const&)>
        ) = 0;
      virtual void reschedule_worker_jobs_and_maybe_remove_worker
        ( fhg::com::p2p::address_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
        , std::function<void (Job* job)>
        ) = 0;
      virtual bool reschedule_job_if_the_reservation_was_canceled
        (job_id_t const& job, status::code) = 0;
      virtual void start_pending_jobs
        (std::function<void ( WorkerSet const&
                            , Implementation const&
                            , job_id_t const&
                            , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
                            )
                      >
        ) = 0;
      virtual void acknowledge_job_sent_to_worker
        (job_id_t const&, fhg::com::p2p::address_t const&) = 0;
    };
  }
}
