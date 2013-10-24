#ifndef SDPA_WORKER_HPP
#define SDPA_WORKER_HPP 1

#include <list>
#include <string>
#include <sdpa/util/util.hpp>
#include <sdpa/common.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace sdpa { namespace daemon {

  /**
    This class holds all information about an attached worker.

    On the orchestrator this represents an aggregator and on the aggregator
    all information about attached NREs is held in this class.
  */

  class Worker {
  public:

    typedef sdpa::shared_ptr<Worker> ptr_t;

    typedef sdpa::location_t location_t;
    typedef sdpa::worker_id_t worker_id_t;
    typedef sdpa::capabilities_set_t capabilities_set_t;

    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;

    typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;

    /**
      A worker has a globally unique name and a location.

      The location can for example represent an ip/port tuple or a queue name etc.
      IDEA: this information should actually be kept in some kind of a
            distributed storage space accessible via the worker's name.

      @param name a unique name for the worker
      @param location how to reach that worker (might be the same as the former)
      */
    explicit Worker( 	const worker_id_t& name = worker_id_t(""),
    					const unsigned int& cap = 10000,
    					const unsigned int& rank = 0,
    					const sdpa::worker_id_t& agent_uuid = "",
    					const location_t &location = "" );

    /**
      Take an event related to that particular worker and update the internal
      data structures.
      */
    void update();

    void submit(const sdpa::job_id_t&);

    /**
	 Acknowledge a given job id and move it to the acknowledged_ queue.

	 @param job_id the job_id to acknowledge
	 @return true iff a job was moved
	 */
    bool acknowledge(const sdpa::job_id_t&);

    // update last service time
    sdpa::util::time_type lastTimeServed() {lock_type lock(mtx_); return last_time_served_; }
    void setLastTimeServed(const sdpa::util::time_type& last_time_srv ) { lock_type lock(mtx_);last_time_served_ = last_time_srv; }

    // update last service time
    sdpa::util::time_type lastScheduleTime() {lock_type lock(mtx_); return last_schedule_time_; }
    void setLastScheduleTime(const sdpa::util::time_type& last_schedule_time ) { lock_type lock(mtx_); last_schedule_time_ = last_schedule_time; }

    /**
      Return the name of the worker.
      */
    const worker_id_t &name() const { lock_type lock(mtx_); return name_; }

    /**
      Return the location of this worker.
      */
    const location_t &location() const { lock_type lock(mtx_); return location_; }

    /**
         Return the rank of the worker.
     */
    unsigned int capacity() const { lock_type lock(mtx_); return capacity_; }
    unsigned int rank() const { lock_type lock(mtx_); return rank_; }
    const sdpa::worker_id_t& agent_uuid() const { lock_type lock(mtx_); return agent_uuid_; }

    // capabilities
    const sdpa::capabilities_set_t& capabilities() const;

    bool addCapabilities(const capabilities_set_t& cpbset);
    void removeCapabilities(const capabilities_set_t& cpbset);
    bool hasCapability(const std::string& cpbName, bool bOwn = false);
    bool hasSimilarCapabilites(const Worker::ptr_t& );

    /**
         Checks if the worker has job
    */
    bool has_job( const sdpa::job_id_t& job_id );
    bool isJobSubmittedOrAcknowleged( const sdpa::job_id_t& job_id );


    /**
		 Return true if the worker is timedout, false otherwise
     */
    bool timedout() const { lock_type lock(mtx_); return timedout_; }
    void set_timedout(bool bValue = true ) { lock_type lock(mtx_); timedout_ = bValue; }

    bool disconnected() const { lock_type lock(mtx_); return disconnected_; }
    void set_disconnected(bool bValue = true) { lock_type lock(mtx_); disconnected_ = bValue; }

    /**
      Remove a job that was finished or failed from the acknowledged_ queue

      a second flag is needed in the case the job is canceled (in order to look into the other queues, as well)
      @param last_job_id the id of the last sucessfully submitted job
    */
    void deleteJob(const sdpa::job_id_t &job_id );

    /**
      Provide access to the submitted queue.

      We are required to have access to the submitted queue of a worker because
      we might need to reschedule tasks.
      */
    JobQueue& submitted() { lock_type lock(mtx_); return submitted_; }
    const JobQueue& submitted() const { lock_type lock(mtx_); return submitted_; }

    /**
      Provide access to the acknowledged queue.

      We are required to have access to the submitted queue of a worker because
      we might need to reschedule tasks.
      */
    JobQueue& acknowledged() { lock_type lock(mtx_); return acknowledged_; }
    const JobQueue& acknowledged() const { lock_type lock(mtx_); return acknowledged_; }

    unsigned int nbAllocatedJobs();

    template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
    	ar & name_;
        ar & rank_;
        ar & location_;
    	ar & tstamp_;
    	ar & last_time_served_;
    	ar & last_schedule_time_;
	}

    friend class boost::serialization::access;

    void print();

    // methods related to reservation
    bool isReserved();
    void reserve();
    void free();

  private:
    SDPA_DECLARE_LOGGER();

    worker_id_t name_; //! name of the worker
    unsigned int capacity_;
    sdpa::capabilities_set_t capabilities_;
    unsigned int rank_;
	sdpa::worker_id_t agent_uuid_;
    location_t location_; //! location where to reach the worker
    sdpa::util::time_type tstamp_; //! time of last message received
    sdpa::util::time_type last_time_served_; //! time of last message received
    sdpa::util::time_type last_schedule_time_;

    JobQueue submitted_; //! the queue of jobs assigned to this worker (sent but not acknowledged)
    JobQueue acknowledged_; //! the queue of jobs assigned to this worker (successfully submitted)

    bool timedout_;
    bool disconnected_;
    bool reserved_;

    mutable mutex_type mtx_;
  };
}}

#endif
