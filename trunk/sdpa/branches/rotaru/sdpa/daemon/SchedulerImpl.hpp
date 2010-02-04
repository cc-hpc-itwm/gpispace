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
#include <sdpa/Sdpa2Gwes.hpp>
#include <sdpa/daemon/IComm.hpp>

namespace sdpa {
	namespace daemon {
  class SchedulerImpl : public Scheduler {

  public:

	 typedef sdpa::shared_ptr<SchedulerImpl> ptr_t;
	 typedef SynchronizedQueue<std::list<Job::ptr_t> > JobQueue;
	 //typedef SynchronizedQueue<Job::ptr_t > JobQueue;

	 SchedulerImpl(sdpa::daemon::IComm* pHandler );
	 virtual ~SchedulerImpl();


	 virtual void schedule(Job::ptr_t& pJob);

	 virtual void schedule(gwes::activity_t& ) { throw std::runtime_error("scheduling of activities not supported here"); }

	 /**
      Scheduling a job locally means that it will not leave the executing node.

      On the Orchestrator that means the initial execution of a workflow,
      resulting activities are then scheduled via the schedule() call.

      @param job a pointer to the job that is to be scheduled locally
      */
	 virtual void schedule_local(const Job::ptr_t &job);

	 /**
      Scheduling a job means that it will be put into a special queue for some worker node (Aggregator or NRE).

      @param job a pointer to the job that shall be executed on a remote node
      */
	 virtual void schedule_remote(const Job::ptr_t &job);

	 virtual void start_job(const Job::ptr_t &pJob);

	 virtual Worker::ptr_t &findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException);
	 virtual void addWorker(const Worker::ptr_t &);
	 virtual size_t numberOfWorkers() { return ptr_worker_man_->numberOfWorkers(); }

	 virtual void check_post_request();
	 virtual bool post_request(bool force = false);
	 virtual void send_life_sign();
	 void set_timeout(long timeout) { m_timeout = boost::posix_time::microseconds(timeout); }

    // thread related functions
	 virtual void start();
	 virtual void stop();
	 virtual void run();

  protected:
	  JobQueue jobs_to_be_scheduled;
	  WorkerManager::ptr_t ptr_worker_man_;

	  bool bStopRequested;
	  boost::thread m_thread;
	  mutable sdpa::daemon::IComm* ptr_comm_handler_;
	  SDPA_DECLARE_LOGGER();
	  boost::posix_time::time_duration m_timeout;
	  sdpa::util::time_type m_last_request_time;
	  sdpa::util::time_type m_last_life_sign_time;
  };
}}

#endif
