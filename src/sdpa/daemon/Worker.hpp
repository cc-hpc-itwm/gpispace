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

#include <list>
#include <string>
#include <sdpa/capability.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <util-generic/refcounted_set.hpp>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>

namespace sdpa
{
  namespace daemon
  {
    class Worker {
    private:
      friend class WorkerManager;

      explicit Worker ( Capabilities const&
                      , unsigned long allocated_shared_memory_size
                      , std::string const& hostname
                      );

      void assign (job_id_t const&, double);
      void submit (job_id_t const&);

      void acknowledge (job_id_t const&);

      // capabilities
      bool hasCapability (std::string const& cpbName) const;

      bool has_pending_jobs() const;
      bool has_running_jobs() const;

      void delete_submitted_job (job_id_t job_id, double);
      void delete_pending_job (job_id_t job_id, double);

      // methods related to reservation
      bool isReserved() const;

      boost::optional<job_id_t> get_next_pending_job_to_submit();

      // cost
      double cost_assigned_jobs() const;
      double _cost_assigned_jobs;

      bool stealing_allowed() const;

      Capabilities _capabilities;
      fhg::util::refcounted_set<std::string> capability_names_;
      unsigned long const _allocated_shared_memory_size;
      std::string const _hostname;
      double _last_time_idle;

      std::set<job_id_t> pending_;
      std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
      std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

      bool reserved_;
    };
  }
}
