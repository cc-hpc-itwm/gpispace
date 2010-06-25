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
#include <sdpa/daemon/IComm.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <tests/sdpa/test_SerializeDaemonComponents.hpp>

namespace sdpa {
	namespace daemon {
  class SchedulerImpl : public Scheduler {

  public:

    typedef sdpa::shared_ptr<SchedulerImpl> ptr_t;
	//typedef SynchronizedQueue<std::list<Job::ptr_t> > JobQueue; //obsolete

	typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;

	SchedulerImpl(sdpa::daemon::IComm* pHandler = NULL);
	virtual ~SchedulerImpl();


	virtual void schedule(sdpa::job_id_t& job);
	virtual void registerHandler(sdpa::daemon::IComm* pHandler) { ptr_comm_handler_ = pHandler; }
	virtual void unregisterHandler(sdpa::daemon::IComm* pHandler) { if(pHandler == ptr_comm_handler_) ptr_comm_handler_=NULL;}

	/**
	Scheduling a job locally means that it will not leave the executing node.

	On the Orchestrator that means the initial execution of a workflow,
	resulting activities are then scheduled via the schedule() call.

	@param job a pointer to the job that is to be scheduled locally
	*/
	virtual void schedule_local(const sdpa::job_id_t &job);

	/**
	Scheduling a job means that it will be put into a special queue for some worker node (Aggregator or NRE).

	@param job a pointer to the job that shall be executed on a remote node
	*/
	virtual void schedule_remote(const sdpa::job_id_t &job);
	void schedule_round_robin(const sdpa::job_id_t &job);
	bool schedule_with_constraints(const sdpa::job_id_t &job);
	bool schedule_to(const sdpa::job_id_t& jobId, unsigned int rank );

	virtual void start_job(const sdpa::job_id_t &job);

	virtual Worker::ptr_t &findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException);
	virtual void addWorker(const Worker::ptr_t &);
	virtual size_t numberOfWorkers() { return ptr_worker_man_->numberOfWorkers(); }
	virtual sdpa::job_id_t getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException);

	virtual void check_post_request();
	virtual bool post_request(bool force = false);
	virtual void send_life_sign();
	void set_timeout(long timeout) { m_timeout = boost::posix_time::microseconds(timeout); }

	// thread related functions
	virtual void start();
	virtual void stop();
	virtual void run();

	template <class Archive>
	void serialize(Archive& ar, const unsigned int file_version )
	{
		ar & boost::serialization::base_object<Scheduler>(*this);
		ar & jobs_to_be_scheduled;
		ar & ptr_worker_man_;
	}

	friend class boost::serialization::access;
	friend class sdpa::tests::WorkerSerializationTest;

	void print();
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
