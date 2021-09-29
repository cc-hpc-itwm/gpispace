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

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/requirements_and_preferences.hpp>

#include <fhgcom/address.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/noncopyable.hpp>
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

namespace sdpa
{
  namespace daemon
  {
    using WorkerSet = std::set<worker_id_t>;
    using Workers_implementation_and_transfer_cost =
      std::tuple<WorkerSet, Implementation, double>;

    class WorkerManager : boost::noncopyable
    {
      typedef std::unordered_map<worker_id_t, Worker> worker_map_t;
      using worker_iterator = worker_map_t::iterator;

    private:
      class WorkerEquivalenceClass
      {
        friend class WorkerManager;

      public:
        WorkerEquivalenceClass();
        WorkerEquivalenceClass (WorkerEquivalenceClass const&) = delete;
        WorkerEquivalenceClass (WorkerEquivalenceClass&&) = delete;
        WorkerEquivalenceClass& operator= (WorkerEquivalenceClass const&) = delete;
        WorkerEquivalenceClass& operator= (WorkerEquivalenceClass const&&) = delete;
        ~WorkerEquivalenceClass() = default;

        void inc_pending_jobs (unsigned int);
        void dec_pending_jobs (unsigned int);
        void inc_running_jobs (unsigned int);
        void dec_running_jobs (unsigned int);

        unsigned int n_pending_jobs() const;
        unsigned int n_running_jobs() const;

        void add_worker_entry (worker_iterator);
        void remove_worker_entry (worker_iterator);

        void allow_classes_matching_preferences_stealing
          ( std::map<std::set<std::string>, WorkerEquivalenceClass> const& worker_classes
          , Preferences const& preferences
          );

      private:
        unsigned int _n_pending_jobs;
        unsigned int _n_running_jobs;
        std::unordered_set<worker_id_t> _idle_workers;
        std::set<std::set<std::string>> _stealing_allowed_classes;
      };

    public:
      std::unordered_set<worker_id_t> find_subm_or_ack_workers
        (sdpa::job_id_t const&, std::set<worker_id_t> const&) const;

      //! throws if workerId was not unique
      void add_worker ( worker_id_t const& workerId
                     , capabilities_set_t const& cpbset
                     , unsigned long allocated_shared_memory_size
                     , std::string const& hostname
                     , fhg::com::p2p::address_t const& address
                     );

      void delete_worker (worker_id_t const& workerId);

      void steal_work (std::function<scheduler::Reservation* (job_id_t const&)> reservation);

      void assign_job_to_worker
        (job_id_t const&, worker_id_t const&, double cost, Preferences const&);
      void submit_job_to_worker (job_id_t const&, worker_id_t const&);
      void acknowledge_job_sent_to_worker (job_id_t const&, worker_id_t const&);
      void delete_job_from_worker (job_id_t const& job_id, worker_id_t const&, double);

      using worker_connections_t
        = boost::bimap < boost::bimaps::unordered_set_of<std::string>
                       , boost::bimaps::unordered_set_of<fhg::com::p2p::address_t>
                       >;

      boost::optional<WorkerManager::worker_connections_t::right_iterator>
        worker_by_address (fhg::com::p2p::address_t const&);

      boost::optional<WorkerManager::worker_connections_t::left_iterator>
        address_by_worker (std::string const&);

      unsigned long num_free_workers() const;
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
    private:
      void assign_job_to_worker
        (job_id_t const&, worker_iterator, double cost, Preferences const&);
      void delete_job_from_worker
        (job_id_t const& job_id, worker_iterator worker, double cost);

      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;
      std::map<std::set<std::string>, WorkerEquivalenceClass> worker_equiv_classes_;
      std::map<std::set<std::string>, std::unordered_set<worker_id_t>> _classes_and_worker_ids;

      unsigned long _num_free_workers {0};
    };
  }
}
