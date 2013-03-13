#ifndef SDPA_SCHEDULERTESTIMPL_HPP
#define SDPA_SCHEDULERTESTIMPL_HPP 1

#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <boost/thread.hpp>
#include <sdpa/daemon/IAgent.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

using namespace sdpa::daemon;

class SchedulerTestImpl : public Scheduler {

  public:

	 typedef sdpa::shared_ptr<SchedulerTestImpl> ptr_t;
	 typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;

	 SchedulerTestImpl();
	 virtual ~SchedulerTestImpl();

    void schedule(sdpa::job_id_t& );
    bool schedule_to(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& );
    void re_schedule(Worker::JobQueue* pQueue );

	virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);
	virtual const Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException);
	virtual void addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException);
	virtual size_t numberOfWorkers() { return ptr_worker_man_->numberOfWorkers(); }
	virtual const sdpa::job_id_t getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException);
	virtual void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException);

	void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException);

	void removeRecoveryInconsistencies(){}

    // thread related functions
    void start();
    void stop();
    void run();

    template <class Archive>
	void serialize(Archive& ar, const unsigned int file_version )
	{
		ar & jobs_to_be_scheduled;
		ar & ptr_worker_man_;
	}

	friend class boost::serialization::access;

	void print();
  private:
	  JobQueue jobs_to_be_scheduled;
	  WorkerManager::ptr_t ptr_worker_man_;

	  bool bStopRequested;
	  boost::thread m_thread;
	  SDPA_DECLARE_LOGGER();
};

#endif
