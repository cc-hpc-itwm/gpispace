/*
 * =====================================================================================
 *
 *       Filename:  SchedulerImpl.hpp
 *
 *    Description:  Defines scheduler class
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_SCHEDULERIMPL_HPP
#define SDPA_SCHEDULERIMPL_HPP 1

#include <boost/thread.hpp>
#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <sdpa/daemon/IAgent.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace sdpa {
  namespace daemon {
	  class SchedulerImpl : public Scheduler
	  {
    public:
      typedef sdpa::shared_ptr<SchedulerImpl> ptr_t;
	    typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
	    typedef boost::recursive_mutex mutex_type;
	    typedef boost::unique_lock<mutex_type> lock_type;
	    typedef boost::condition_variable_any condition_type;

	    SchedulerImpl(sdpa::daemon::IAgent* pHandler = NULL, bool bUseRequestModel = true );
	    virtual ~SchedulerImpl();

	    virtual void schedule(const sdpa::job_id_t&);
	    virtual void schedule_local(const sdpa::job_id_t&);
	    virtual void schedule_remote(const sdpa::job_id_t&);
	    void delete_job(const sdpa::job_id_t&);

	    //bool schedule_with_constraints( const sdpa::job_id_t& );
	    bool schedule_to( const sdpa::job_id_t&, const sdpa::worker_id_t& );
	    bool schedule_to( const sdpa::job_id_t&, const Worker::ptr_t& pWorker );
	    void dispatch( const sdpa::job_id_t& jobId );

	    sdpa::list_match_workers_t find_matching_workers( const sdpa::job_id_t& jobId );

	    void reschedule(const sdpa::job_id_t &job);
	    void reschedule( const Worker::worker_id_t &, Worker::JobQueue* pQueue);
	    void reschedule( const Worker::worker_id_t& worker_id );
	    void reschedule( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id );
	    void reassign( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id );

	    virtual bool has_job(const sdpa::job_id_t&);

	    virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t&) throw (NoWorkerFoundException);
	    virtual const Worker::ptr_t& findWorker(const Worker::worker_id_t&) throw(WorkerNotFoundException);
	    virtual const Worker::worker_id_t& findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

	    virtual void addWorker( const Worker::worker_id_t& workerId,
	    						const unsigned int& capacity = 10000,
			                    const capabilities_set_t& cpbset = capabilities_set_t(),
			                    const unsigned int& agent_rank = 0,
			                    const sdpa::worker_id_t& agent_uuid = "") throw (WorkerAlreadyExistException);

	    virtual void delWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException);
	    void declare_jobs_failed( const Worker::worker_id_t&, Worker::JobQueue* pQueue );

	    virtual void getWorkerList(sdpa::worker_id_list_t&);
	    void getListWorkersNotFull(sdpa::worker_id_list_t& workerList);
	    virtual Worker::worker_id_t getWorkerId(unsigned int rank);

	    virtual void setLastTimeServed(const worker_id_t& wid, const sdpa::util::time_type& servTime);
	    virtual size_t numberOfWorkers() { return ptr_worker_man_->numberOfWorkers(); }

	    virtual bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
	    virtual void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException);
	    virtual void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
	    virtual void getWorkerCapabilities(const sdpa::worker_id_t&, sdpa::capabilities_set_t& cpbset);

	    virtual const sdpa::job_id_t assignNewJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException);
	    const sdpa::job_id_t assignNewJob(const Worker::ptr_t ptrWorker, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException);
	    virtual void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException);

	    virtual void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException);

	    virtual void execute(const sdpa::job_id_t& jobId); //just for testing
	    virtual void check_post_request();
	    virtual bool post_request(const MasterInfo& masterInfo, bool force = false);

	    virtual void feedWorkers();

	    void cancelWorkerJobs();
	    void planForCancellation(const Worker::worker_id_t& workerId, const sdpa::job_id_t& jobId);
	    virtual void forceOldWorkerJobsTermination();

	    virtual bool useRequestModel() { return m_bUseRequestModel; }
	    void setUseRequestModel (bool b) { m_bUseRequestModel = b; }

	    void set_timeout(long timeout) { m_timeout = boost::posix_time::microseconds(timeout); }

    	// thread related functions
	    virtual void start(IAgent*);
	    virtual void stop();
	    virtual void run();

	    template <class Archive>
	    void serialize(Archive& ar, const unsigned int)
	    {
		    ar & boost::serialization::base_object<Scheduler>(*this);
		    ar & ptr_worker_man_;
	    }

	    friend class boost::serialization::access;

	    virtual void print();
	    virtual void removeRecoveryInconsistencies();
        void removeWorkers() { ptr_worker_man_->removeWorkers(); }

        void printQ() { jobs_to_be_scheduled.print(); }

    protected:
	    JobQueue jobs_to_be_scheduled;
	    WorkerManager::ptr_t ptr_worker_man_;

	    bool bStopRequested;
	    boost::thread m_thread_run;
	    boost::thread m_thread_feed;

	    mutable sdpa::daemon::IAgent* ptr_comm_handler_;
	    SDPA_DECLARE_LOGGER();
	    boost::posix_time::time_duration m_timeout;

	    bool m_bUseRequestModel; // true -> request model, false -> push model
	    sdpa::cancellation_list_t cancellation_list_;

	    mutable mutex_type mtx_;
	    condition_type cond_feed_workers;
	    condition_type cond_workers_registered;
    };
  }
}

#endif
