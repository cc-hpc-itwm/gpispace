#pragma once

#include <list>
#include <string>
#include <sdpa/capability.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>

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
                      , unsigned long allocated_shared_memory_size
                      , const bool children_allowed
                      , const std::string& hostname
                      , const fhg::com::p2p::address_t& address
                      );

      void assign (const job_id_t&);
      void submit(const job_id_t&);

      void acknowledge(const job_id_t&);

      double lastTimeServed() const { return last_time_served_; }

      std::string hostname() const { return hostname_; }
      fhg::com::p2p::address_t address() const;

      // capabilities
      const capabilities_set_t& capabilities() const;

      bool addCapabilities(const capabilities_set_t& cpbset);
      bool removeCapabilities(const capabilities_set_t& cpbset);
      bool hasCapability(const std::string& cpbName) const;

      bool has_job( const job_id_t& job_id ) const;
      bool has_pending_jobs() const;

      void deleteJob(const job_id_t &job_id );

      // methods related to reservation
      bool isReserved() const;
      bool backlog_full() const;
      void set_backlog_full (bool);

      // cost
      double cost_assigned_jobs (std::function<double (job_id_t job_id)>) const;

      bool remove_job_if_pending (const job_id_t& job_id);

      std::set<job_id_t> getJobListAndCleanQueues();

      capabilities_set_t capabilities_;
      unsigned long const _allocated_shared_memory_size;
      bool const _children_allowed;
      std::string hostname_;
      fhg::com::p2p::address_t address_;
      double last_time_served_;

      std::set<job_id_t> pending_;
      std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
      std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

      bool reserved_;
      bool backlog_full_;
    };
  }
}
