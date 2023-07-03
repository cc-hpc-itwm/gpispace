// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/capability.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <util-generic/refcounted_set.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

#include <list>
#include <string>

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
      double _cost_assigned_jobs {0};

      bool stealing_allowed() const;

      Capabilities _capabilities;
      fhg::util::refcounted_set<std::string> capability_names_;
      unsigned long const _allocated_shared_memory_size;
      std::string const _hostname;
      double _last_time_idle;

      std::set<job_id_t> pending_;
      std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
      std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

      bool reserved_ {false};
    };
  }
}
