#ifndef SDPA_WORKER_HPP
#define SDPA_WORKER_HPP 1

#include <list>
#include <string>
#include <sdpa/capability.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/optional.hpp>

namespace sdpa
{
  namespace daemon
  {
    class Worker {
    public:

      typedef boost::shared_ptr<Worker> ptr_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      explicit Worker( 	const worker_id_t& name,
                                          const boost::optional<unsigned int>& cap
                     , const capabilities_set_t&
                     , const bool children_allowed);

      void submit(const job_id_t&);

      void acknowledge(const job_id_t&);

      // update last service time
      double lastScheduleTime() {lock_type lock(mtx_); return last_schedule_time_; }

      const worker_id_t &name() const { lock_type lock(mtx_); return name_; }

      boost::optional<unsigned int> capacity() const { lock_type lock(mtx_); return capacity_; }

      // capabilities
      const capabilities_set_t& capabilities() const;
      const bool children_allowed() const { return children_allowed_;}

      bool addCapabilities(const capabilities_set_t& cpbset);
      void removeCapabilities(const capabilities_set_t& cpbset);
      bool hasCapability(const std::string& cpbName);

      bool has_job( const job_id_t& job_id );

      void deleteJob(const job_id_t &job_id );

      // methods related to reservation
      bool isReserved();
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
      double last_schedule_time_;

      std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
      std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

      bool reserved_;

      mutable mutex_type mtx_;
    };
  }
}

#endif
