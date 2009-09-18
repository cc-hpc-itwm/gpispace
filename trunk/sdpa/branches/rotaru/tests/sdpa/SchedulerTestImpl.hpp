#ifndef SDPA_SCHEDULERIMPL_HPP
#define SDPA_SCHEDULERIMPL_HPP 1

#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <boost/thread.hpp>

using namespace sdpa::daemon;

class SchedulerTestImpl : public Scheduler {

  public:

	 typedef sdpa::shared_ptr<SchedulerTestImpl> ptr_t;
	 typedef SynchronizedQueue<std::list<Job::ptr_t> > JobQueue;

	 SchedulerTestImpl();
	 virtual ~SchedulerTestImpl();

    void handleJob(Job::ptr_t& pJob);
    Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException);
    void addWorker(const  Worker::ptr_t& );

    // thread related functions
    void start();
    void stop();
    void run();


  private:
	  JobQueue jobs_to_be_scheduled;

	  bool bStopRequested;
	  boost::thread m_thread;
	  SDPA_DECLARE_LOGGER();
};

#endif
