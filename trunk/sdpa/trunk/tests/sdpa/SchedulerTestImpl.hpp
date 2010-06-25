#ifndef SDPA_SCHEDULERTESTIMPL_HPP
#define SDPA_SCHEDULERTESTIMPL_HPP 1

#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <boost/thread.hpp>
#include <sdpa/daemon/IComm.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <tests/sdpa/test_SerializeDaemonComponents.hpp>
#include <boost/serialization/shared_ptr.hpp>

using namespace sdpa::daemon;

class SchedulerTestImpl : public Scheduler {

  public:

	 typedef sdpa::shared_ptr<SchedulerTestImpl> ptr_t;
	 typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;

	 SchedulerTestImpl();
	 virtual ~SchedulerTestImpl();

    void schedule(sdpa::job_id_t& );

    Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException);
    void addWorker(const  Worker::ptr_t& );
    size_t numberOfWorkers() { return 1; }
    virtual sdpa::job_id_t getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException);

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
	friend class sdpa::tests::WorkerSerializationTest;

	void print();
  private:
	  JobQueue jobs_to_be_scheduled;
	  WorkerManager::ptr_t ptr_worker_man_;

	  bool bStopRequested;
	  boost::thread m_thread;
	  SDPA_DECLARE_LOGGER();
};

#endif
