#ifndef SDPA_WORKER_HPP
#define SDPA_WORKER_HPP 1

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
    public:

      typedef boost::shared_ptr<Worker> ptr_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      explicit Worker ( const worker_id_t& name
                      , const boost::optional<unsigned int>& cap
                      , const capabilities_set_t&
                      , const bool children_allowed
                      , const std::string& hostname
                      , const fhg::com::p2p::address_t& address
                      );

      void assign (const job_id_t&);
      void submit(const job_id_t&);

      void acknowledge(const job_id_t&);

      double lastTimeServed() {lock_type lock(mtx_); return last_time_served_; }

      const worker_id_t &name() const { lock_type lock(mtx_); return name_; }
      const std::string hostname() const;
      fhg::com::p2p::address_t address() const;
      boost::optional<unsigned int> capacity() const { lock_type lock(mtx_); return capacity_; }

      // capabilities
      const capabilities_set_t& capabilities() const;
      bool children_allowed() const { return children_allowed_;}

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
    private:
      void reserve();
      void free();
    public:

      std::set<job_id_t> getJobListAndCleanQueues();

    private:
      worker_id_t name_; //! name of the worker
      boost::optional<unsigned int> capacity_;
      capabilities_set_t capabilities_;
      bool children_allowed_;
      std::string hostname_;
      fhg::com::p2p::address_t address_;
      double last_time_served_;

      std::set<job_id_t> pending_;
      std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
      std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

      bool reserved_;
      bool backlog_full_;

      mutable mutex_type mtx_;
    };
  }
}

#endif
