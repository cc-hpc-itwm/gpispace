// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/daemon/scheduler/Scheduler.hpp>
#include <sdpa/requirements_and_preferences.hpp>
#include <sdpa/types.hpp>

#include <fhgcom/address.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>

namespace sdpa
{
  namespace daemon
  {
    struct CostsAndMatchingWorkerInfo
    {
      CostsAndMatchingWorkerInfo
          ( double cost
          , double matching_degree
          , unsigned long shared_memory_size
          , double last_time_idle
          , worker_id_t const& worker_id
          , ::boost::optional<std::string> const& implementation
          , double transfer_cost
          , std::set<std::string> const& worker_class
          )
        : _cost (cost)
        , _matching_degree (matching_degree)
        , _shared_memory_size (shared_memory_size)
        , _last_time_idle (last_time_idle)
        , _worker_id (worker_id)
        , _implementation (implementation)
        , _transfer_cost (transfer_cost)
        , _worker_class (worker_class)
      {}

      double _cost;
      double _matching_degree;
      unsigned long _shared_memory_size;
      double _last_time_idle;
      worker_id_t _worker_id;
      ::boost::optional<std::string> _implementation;
      double _transfer_cost;
      std::set<std::string> _worker_class;
    };

    class Compare
    {
    public:
      bool operator()
        ( CostsAndMatchingWorkerInfo const& lhs
        , CostsAndMatchingWorkerInfo const& rhs
        ) const
      {
      #define ESSENCE(x) std::tie (x._cost, x._matching_degree, x._shared_memory_size, x._last_time_idle)
        return ESSENCE (lhs) < ESSENCE (rhs);
      #undef ESSENCE
      }
    };

    //! \todo how is this class related to
    //! gspc_nextgen/workflow_engine/ProcessingState?
    class CostAwareWithWorkStealingStrategy
    {
    public:
      CostAwareWithWorkStealingStrategy() = delete;
      CostAwareWithWorkStealingStrategy
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
        , WorkerManager&
        );
      CostAwareWithWorkStealingStrategy
        (CostAwareWithWorkStealingStrategy const&) = delete;
      CostAwareWithWorkStealingStrategy
        (CostAwareWithWorkStealingStrategy&&) = delete;
      CostAwareWithWorkStealingStrategy& operator=
        (CostAwareWithWorkStealingStrategy const&) = delete;
      CostAwareWithWorkStealingStrategy& operator=
        (CostAwareWithWorkStealingStrategy&&) = delete;
      ~CostAwareWithWorkStealingStrategy() = default;

      void submit_job (sdpa::job_id_t const&);
      void acknowledge_job_sent_to_worker
        (job_id_t const&, fhg::com::p2p::address_t const&);
      ::boost::optional<job_result_type>
        store_individual_result_and_get_final_if_group_finished
          ( fhg::com::p2p::address_t const&
          , job_id_t const&
          , terminal_state const&
          );
      void steal_work (CostModel const&);
      void delete_job (sdpa::job_id_t const&);
      std::pair<::boost::optional<double>,::boost::optional<std::string>>
        match_requirements_and_preferences
          ( Requirements_and_preferences const&
          , std::set<std::string> const&
          ) const;
      void notify_submitted_or_acknowledged_workers
        ( job_id_t const&
        , std::function<void ( fhg::com::p2p::address_t const&)>
        );
      void release_reservation
        ( sdpa::job_id_t const& job_id
        , std::lock_guard<std::mutex> const&
        , std::lock_guard<std::mutex> const&
        );
      void reschedule_worker_jobs_and_maybe_remove_worker
        ( fhg::com::p2p::address_t const& source
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (Job* job)>
        );
      void reschedule_worker_jobs_and_maybe_remove_worker
        ( fhg::com::p2p::address_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)>
        , std::function<void (Job* job)>
        );
      bool cancel_job
        (job_id_t const&, std::function<void (fhg::com::p2p::address_t const&)>);
      bool cancel_job
        ( job_id_t const&
        , bool
        , std::function<void (fhg::com::p2p::address_t const&)>
        );

      bool assign_job
        ( job_id_t const&
        , WorkerSet const&
        , double
        , Implementation const&
        , Preferences const&
        , std::list<job_id_t>&
        , std::lock_guard<std::mutex> const& lock_alloc_table
        , std::lock_guard<std::mutex> const& lock_worker_man
        );

      friend class SingleAllocationScheduler;
      friend class CoallocationScheduler;
      friend class GreedyScheduler;

    protected:
      std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
        _requirements_and_preferences;

      std::unordered_set<sdpa::job_id_t> delete_or_cancel_worker_jobs
        ( worker_id_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (fhg::com::p2p::address_t const&, job_id_t const&)>
        , std::lock_guard<std::mutex> const&
        , std::lock_guard<std::mutex> const&
        );

      WorkerManager& _worker_manager;

      class locked_job_id_list
      {
      public:
        void push (job_id_t const&);
        void push (std::list<job_id_t> const&);
        size_t erase (job_id_t const&);
        std::list<job_id_t> get_and_clear();
      private:
        std::mutex mtx_;
        std::list<job_id_t> container_;
      } _jobs_to_schedule;

      std::mutex mtx_alloc_table_;
      using allocation_table_t
        = std::unordered_map<job_id_t, std::unique_ptr<scheduler::Reservation>>;
      allocation_table_t allocation_table_;

      class PendingJobs
      {
      public:
        void add (std::set<std::string>, job_id_t);
        void remove (job_id_t);
        std::map <std::set<std::string>, std::unordered_set<job_id_t>>&
          operator()();
      private:
        std::map <std::set<std::string>, std::unordered_set<job_id_t>> _jobs;
        std::unordered_map<job_id_t, std::set<std::string>> _class;
      };

      PendingJobs _pending_jobs;

      friend class access_allocation_table_TESTING_ONLY;
    };
  }
}
