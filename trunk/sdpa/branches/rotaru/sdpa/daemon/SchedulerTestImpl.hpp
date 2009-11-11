#ifndef SDPA_SCHEDULERTESTIMPL_HPP
#define SDPA_SCHEDULERTESTIMPL_HPP 1

#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <boost/thread.hpp>


#include <sdpa/Sdpa2Gwes.hpp>

namespace sdpa { namespace daemon {

class SchedulerTestImpl : public Scheduler {

  public:

	 typedef sdpa::shared_ptr<SchedulerTestImpl> ptr_t;
	 typedef SynchronizedQueue<std::list<Job::ptr_t> > JobQueue;

	 SchedulerTestImpl(sdpa::Sdpa2Gwes* ptr_Sdpa2Gwes=NULL);
	 virtual ~SchedulerTestImpl();

    void schedule(Job::ptr_t& pJob);
    virtual void schedule(gwes::activity_t::ptr_t& pAct) {};
    Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException);
    void addWorker(const  Worker::ptr_t& );

    int numberOfWorkers() { return 1; }
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
}}
#endif
