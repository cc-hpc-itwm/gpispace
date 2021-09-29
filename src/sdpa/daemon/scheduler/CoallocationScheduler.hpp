// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
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
    class CoallocationScheduler : boost::noncopyable
    {
    public:
      CoallocationScheduler
        (std::function<Requirements_and_preferences (sdpa::job_id_t const&)>);

      // -- used by daemon
      void delete_job (sdpa::job_id_t const&);

      void store_result
        (fhg::com::p2p::address_t const&, job_id_t const&, terminal_state);
      boost::optional<job_result_type>
        get_aggregated_results_if_all_terminated (job_id_t const&);

      // -- used by daemon and self
      void submit_job (sdpa::job_id_t const&);

      // used by daemon and self and test
      void release_reservation (sdpa::job_id_t const&);
      void assign_jobs_to_workers();
      void steal_work();

      bool cancel_job_and_siblings
        ( job_id_t const&
        , bool
        , std::function<void (fhg::com::p2p::address_t const&)>
        );
      void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void ( fhg::com::p2p::address_t const&)>
        );
      void reschedule_worker_jobs_and_maybe_remove_worker
        ( fhg::com::p2p::address_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
        );

      bool reschedule_job_if_the_reservation_was_canceled
        (job_id_t const& job, status::code const);

      void start_pending_jobs
        (std::function<void ( WorkerSet const&
                            , Implementation const&
                            , job_id_t const&
                            , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
                            )
                      >
        );

      void add_worker ( worker_id_t const&
                      , capabilities_set_t const&
                      , unsigned long
                      , std::string const&
                      , fhg::com::p2p::address_t const&
                      );

      void delete_worker_TESTING_ONLY (worker_id_t const&);

      void acknowledge_job_sent_to_worker
        (job_id_t const&, fhg::com::p2p::address_t const&);

    private:
      void release_reservation
        ( sdpa::job_id_t const&
        , std::lock_guard<std::mutex> const&
        , std::lock_guard<std::mutex> const&
        );

      std::unordered_set<sdpa::job_id_t> delete_or_cancel_worker_jobs
        ( worker_id_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const&, job_id_t const&)>
        , std::lock_guard<std::mutex> const&
        , std::lock_guard<std::mutex> const&
        );

      Workers_implementation_and_transfer_cost find_assignment
        ( Requirements_and_preferences const&
        , std::lock_guard<std::mutex> const&
        ) const;

      std::pair<boost::optional<double>, boost::optional<std::string>>
        match_requirements_and_preferences
          ( Requirements_and_preferences const&
          , std::set<std::string> const&
          ) const;

      std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
        _requirements_and_preferences;

      std::mutex _mtx_worker_man;
      WorkerManager _worker_manager;

      class locked_job_id_list
      {
      public:
        inline void push (job_id_t const& item);
        template <typename Range>
        inline void push (Range const& items);
        inline size_t erase (job_id_t const& item);

        std::list<job_id_t> get_and_clear();

      private:
        std::mutex mtx_;
        std::list<job_id_t> container_;
      } _jobs_to_schedule;

      std::mutex mtx_alloc_table_;
      using allocation_table_t
        = std::unordered_map<job_id_t, std::unique_ptr<scheduler::Reservation>>;
      allocation_table_t allocation_table_;

      std::unordered_set<job_id_t> _pending_jobs;

      friend class access_allocation_table_TESTING_ONLY;
    };
  }
}
