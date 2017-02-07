#pragma once

#include <list>
#include <string>
#include <sdpa/capability.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <util-generic/refcounted_set.hpp>

#include <vmem/types.hpp>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>

namespace sdpa
{
  namespace daemon
  {
    class Worker {
    private:
      friend class WorkerManager;

      explicit Worker ( const capabilities_set_t&
                      , boost::optional<intertwine::vmem::size_t>
                      , boost::optional<intertwine::vmem::rank_t>
                      , const bool children_allowed
                      );

      void assign (const job_id_t&);
      void submit(const job_id_t&);

      void acknowledge(const job_id_t&);

      // capabilities
      bool addCapabilities(const capabilities_set_t& cpbset);
      bool removeCapabilities(const capabilities_set_t& cpbset);
      bool hasCapability(const std::string& cpbName) const;

      bool has_pending_jobs() const;
      bool has_running_jobs() const;

      void delete_submitted_job (const job_id_t &job_id);
      void delete_pending_job (const job_id_t& job_id);

      // methods related to reservation
      bool isReserved() const;
      bool backlog_full() const;
      void set_backlog_full (bool);

      // cost
      double cost_assigned_jobs (std::function<double (job_id_t job_id)>) const;

      bool stealing_allowed() const;

      capabilities_set_t _capabilities;
      fhg::util::refcounted_set<std::string> capability_names_;
      boost::optional<intertwine::vmem::size_t> vmem_cache_size;
      boost::optional<intertwine::vmem::rank_t> vmem_rank;
      bool const _children_allowed;
      double _last_time_idle;

      std::set<job_id_t> pending_;
      std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
      std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

      bool reserved_;
      bool backlog_full_;
    };
  }
}
