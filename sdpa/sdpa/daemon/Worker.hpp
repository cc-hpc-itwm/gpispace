#ifndef SDPA_WORKER_HPP
#define SDPA_WORKER_HPP 1

#include <list>
#include <string>
#include <sdpa/capability.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>

namespace sdpa { namespace daemon {

  /**
    This class holds all information about an attached worker.

    On the orchestrator this represents an aggregator and on the aggregator
    all information about attached NREs is held in this class.
  */

  class Worker {
  public:

    typedef boost::shared_ptr<Worker> ptr_t;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

    /**
      A worker has a globally unique name.

      @param name a unique name for the worker
     */
    explicit Worker ( const worker_id_t&
                    , const boost::optional<unsigned int>&
                    , const capabilities_set_t&
                    );

    void submit (const job_id_t&);

    /**
	 Acknowledge a given job id and move it to the acknowledged_ queue.

	 @param job_id the job_id to acknowledge
	 */
    void acknowledge (const job_id_t&);

    // update last service time
    double lastScheduleTime() {lock_type lock(mtx_); return last_schedule_time_; }

    /**
      Return the name of the worker.
    */
    const worker_id_t &name() const { lock_type lock(mtx_); return name_; }

    /**
       Return the host name (the worker name already contains the name of the host!)
    */
    const std::string hostname (const std::string& worker_id) const;
    /**
         Return the rank of the worker.
     */
    boost::optional<unsigned int> capacity() const { lock_type lock(mtx_); return capacity_; }

    // capabilities
    const capabilities_set_t& capabilities() const;

    bool addCapabilities (const capabilities_set_t& cpbset);
    void removeCapabilities (const capabilities_set_t& cpbset);
    bool hasCapability (const std::string& cpbName);

    /**
         Checks if the worker has job
    */
    bool has_job (const job_id_t& job_id);


    /**
      Remove a job that was finished or failed from the acknowledged_ queue

      a second flag is needed in the case the job is canceled (in order to look into the other queues, as well)
      @param last_job_id the id of the last sucessfully submitted job
    */
    void deleteJob (const job_id_t &job_id );

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
    double last_schedule_time_;

    std::set<job_id_t> submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
    std::set<job_id_t> acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

    bool reserved_;

    mutable mutex_type mtx_;
  };
}}

#endif
