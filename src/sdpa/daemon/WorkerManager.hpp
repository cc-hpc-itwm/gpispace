// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/requirements_and_preferences.hpp>

#include <fhgcom/address.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/optional.hpp>

#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

namespace sdpa
{
  namespace daemon
  {
    struct UsingCosts{};
    struct NotUsingCosts{};
    using CostModel = std::variant<UsingCosts, NotUsingCosts>;

    class WorkerManager
    {
      using worker_map_t = std::unordered_map<worker_id_t, Worker>;
      using worker_iterator = worker_map_t::iterator;

    private:
      class WorkerEquivalenceClass
      {
        friend class WorkerManager;

      public:
        WorkerEquivalenceClass() = default;
        WorkerEquivalenceClass (WorkerEquivalenceClass const&) = delete;
        WorkerEquivalenceClass (WorkerEquivalenceClass&&) = delete;
        WorkerEquivalenceClass& operator= (WorkerEquivalenceClass const&) = delete;
        WorkerEquivalenceClass& operator= (WorkerEquivalenceClass const&&) = delete;
        ~WorkerEquivalenceClass() = default;

        void inc_pending_jobs (unsigned int);
        void dec_pending_jobs (unsigned int);
        void inc_running_jobs (unsigned int);
        void dec_running_jobs (unsigned int);

        unsigned int num_pending_jobs() const;
        unsigned int num_running_jobs() const;

        void add_worker_entry (worker_iterator);
        void remove_worker_entry (worker_iterator);

        void allow_classes_matching_preferences_stealing
          ( std::map<std::set<std::string>, WorkerEquivalenceClass> const& worker_classes
          , Preferences const& preferences
          );

      private:
        unsigned int _num_pending_jobs {0};
        unsigned int _num_running_jobs {0};
        unsigned long _num_free_workers {0};
        std::unordered_set<worker_id_t> _idle_workers;
        std::set<std::set<std::string>> _stealing_allowed_classes;
      };

    public:
      WorkerManager() = default;
      ~WorkerManager() = default;
      WorkerManager (WorkerManager const&) = delete;
      WorkerManager (WorkerManager&&) = delete;
      WorkerManager& operator= (WorkerManager const&) = delete;
      WorkerManager& operator= (WorkerManager&&) = delete;

      std::unordered_set<worker_id_t> find_subm_or_ack_workers
        (sdpa::job_id_t const&, std::set<worker_id_t> const&) const;

      //! throws if workerId was not unique
      void add_worker ( worker_id_t const& workerId
                      , Capabilities const& cpbset
                      , unsigned long allocated_shared_memory_size
                      , std::string const& hostname
                      , fhg::com::p2p::address_t const& address
                      );

      void delete_worker (worker_id_t const& workerId);

      void steal_work (CostModel, std::function<scheduler::Reservation* (job_id_t const&)>);

      void assign_job_to_worker
        (job_id_t const&, worker_id_t const&, double cost, Preferences const&);
      void submit_job_to_worker (job_id_t const&, worker_id_t const&);
      void acknowledge_job_sent_to_worker (job_id_t const&, worker_id_t const&);
      void delete_job_from_worker (job_id_t const& job_id, worker_id_t const&, double);

      using worker_connections_t
        = ::boost::bimap < ::boost::bimaps::unordered_set_of<std::string>
                       , ::boost::bimaps::unordered_set_of<fhg::com::p2p::address_t>
                       >;

      ::boost::optional<WorkerManager::worker_connections_t::right_iterator>
        worker_by_address (fhg::com::p2p::address_t const&);

      ::boost::optional<WorkerManager::worker_connections_t::left_iterator>
        address_by_worker (std::string const&);

      unsigned long num_free_workers (std::set<std::string> const&) const;
      bool all_free (std::set<worker_id_t> const&) const;

      std::unordered_set<job_id_t> pending_jobs (worker_id_t const& worker) const;
      std::unordered_set<job_id_t> submitted_or_acknowledged_jobs
        (worker_id_t const& worker) const;

      std::tuple<double, unsigned long, double, double>
        costs_memory_size_and_last_idle_time
          (worker_id_t const&, Requirements_and_preferences const&) const;
      unsigned long number_of_workers() const;
      std::map<std::set<std::string>, std::unordered_set<worker_id_t>> const&
        classes_and_workers() const;

      boost::optional<job_id_t> get_next_worker_pending_job_to_submit (worker_id_t const&);
      unsigned long num_running_jobs (std::set<std::string> const&);
      unsigned long num_pending_jobs (std::set<std::string> const&);

      //! Note: the worker manager is shared between the agent and the scheduler.
      // Before accessing it, this mutex should be locked.
      mutable std::mutex _mutex;
    private:
      void assign_job_to_worker
        (job_id_t const&, worker_iterator, double cost, Preferences const&);
      void delete_job_from_worker
        (job_id_t const& job_id, worker_iterator worker, double cost);

      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;
      std::map<std::set<std::string>, WorkerEquivalenceClass> worker_equiv_classes_;
      std::map<std::set<std::string>, std::unordered_set<worker_id_t>> _classes_and_worker_ids;
    };
  }
}
