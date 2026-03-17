// Copyright (C) 2010-2011,2013-2016,2019-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/daemon/Worker.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <sys/time.h>


  namespace gspc::scheduler::daemon
  {
    namespace
    {
      double now()
      {
        struct timeval tv;

        gettimeofday (&tv, nullptr);

        return (double (tv.tv_sec) + double (tv.tv_usec) * 1E-6);
      }
    }

    Worker::Worker ( Capabilities const& capabilities
                   , unsigned long allocated_shared_memory_size
                   , std::string const& hostname
                   )
      : _capabilities (capabilities)
      , capability_names_()
      , _allocated_shared_memory_size (allocated_shared_memory_size)
      , _hostname (hostname)
      , _last_time_idle (now())
    {
      for (Capability const& capability : capabilities)
      {
        capability_names_.emplace (capability.name());
      }
    }

    bool Worker::has_pending_jobs() const
    {
      return !pending_.empty();
    }

    bool Worker::has_running_jobs() const
    {
      return !submitted_.empty() || !acknowledged_.empty();
    }

    void Worker::assign (job_id_t const& jobId, double cost)
    {
      pending_.insert (jobId);
      _cost_assigned_jobs += cost;
    }

    void Worker::submit (job_id_t const& jobId)
    {
      if (!pending_.erase (jobId))
      {
        throw std::runtime_error ("subnmit: no pending job with the id " + jobId + " was found!");
      }
      submitted_.insert (jobId);
      reserved_ = true;
    }

    void Worker::acknowledge (job_id_t const& job_id)
    {
      if (submitted_.erase (job_id) == 0)
      {
        throw std::runtime_error ("acknowledge: job not in submitted queue");
      }
      acknowledged_.insert (job_id);
    }

    void Worker::delete_submitted_job (job_id_t job_id, double cost)
    {
      submitted_.erase (job_id);
      acknowledged_.erase (job_id);
      _last_time_idle = now();
      reserved_ = false;

      _cost_assigned_jobs -= cost;
    }

    void Worker::delete_pending_job (job_id_t job_id, double cost)
    {
      if (0 == pending_.erase (job_id))
      {
        throw std::runtime_error ( "Could not remove the pending job "
                                 + job_id
                                 );
      }

      _cost_assigned_jobs -= cost;
    }

    bool Worker::hasCapability (std::string const& cpbName) const
    {
      return capability_names_.contains (cpbName);
    }

    bool Worker::isReserved() const
    {
      return reserved_;
    }

    double Worker::cost_assigned_jobs() const
    {
      return _cost_assigned_jobs;
    }

    bool Worker::stealing_allowed() const
    {
      return ( (has_pending_jobs() && has_running_jobs())
            || (pending_.size() > 1)
             );
    }

    std::optional<job_id_t> Worker::get_next_pending_job_to_submit()
    {
      return (has_pending_jobs() && !has_running_jobs())
        ? std::make_optional<job_id_t> (*pending_.cbegin())
        : std::nullopt
        ;
    }
  }
