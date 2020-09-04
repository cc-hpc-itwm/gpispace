// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <functional>
#include <mutex>

namespace sdpa
{
  namespace daemon
  {
    class CoallocationScheduler : boost::noncopyable
    {
    public:
      CoallocationScheduler
        ( std::function<Requirements_and_preferences (const sdpa::job_id_t&)>
        , WorkerManager&
        );

      // -- used by daemon
      bool delete_job (const sdpa::job_id_t&);
      void delete_pending_job (const sdpa::job_id_t&);

      void store_result (worker_id_t const&, job_id_t const&, terminal_state);
      boost::optional<job_result_type>
        get_aggregated_results_if_all_terminated (job_id_t const&);

      // -- used by daemon and self
      void enqueueJob (const sdpa::job_id_t&);
      void request_scheduling();

      // used by daemon and self and test
      void releaseReservation (const sdpa::job_id_t&);
      void assignJobsToWorkers();
      void steal_work();

      void reschedule_worker_jobs_and_maybe_remove_worker
        ( worker_id_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (sdpa::worker_id_t const&, job_id_t const&)>
        , bool
        );

      std::set<job_id_t> start_pending_jobs
        (std::function<void ( WorkerSet const&
                            , Implementation const&
                            , const job_id_t&
                            )
                      >
        );

      bool reservation_canceled (job_id_t const&) const;
    private:
      double compute_reservation_cost
        ( const job_id_t&
        , const std::set<worker_id_t>&
        , const double computational_cost
        ) const;

      std::function<Requirements_and_preferences (const sdpa::job_id_t&)>
        _requirements_and_preferences;

      WorkerManager& _worker_manager;

      class locked_job_id_list
      {
      public:
        inline void push (job_id_t const& item);
        template <typename Range>
        inline void push (Range const& items);
        inline size_t erase (const job_id_t& item);

        std::list<job_id_t> get_and_clear();

      private:
        mutable std::mutex mtx_;
        std::list<job_id_t> container_;
      } _jobs_to_schedule;

      //! \note to be able to call releaseReservation instead of
      //! reimplementing in reschedule_worker_jobs
      mutable std::recursive_mutex mtx_alloc_table_;
      using allocation_table_t
        = std::unordered_map<job_id_t, std::unique_ptr<scheduler::Reservation>>;
      allocation_table_t allocation_table_;

      std::unordered_set<job_id_t> _pending_jobs;

      friend class access_allocation_table_TESTING_ONLY;
    };
  }
}
