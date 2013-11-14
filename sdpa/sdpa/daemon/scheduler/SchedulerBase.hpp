/*
 * =====================================================================================
 *
 *       Filename:  SchedulerBase.hpp
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
#ifndef SDPA_SchedulerBase_HPP
#define SDPA_SchedulerBase_HPP 1

#include <boost/thread.hpp>
#include <sdpa/daemon/scheduler/Scheduler.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/SynchronizedQueue.hpp>
#include <sdpa/daemon/IAgent.hpp>

#include <boost/optional.hpp>

namespace sdpa {
  namespace daemon {
    class SchedulerBase : public Scheduler
    {
    public:
      typedef sdpa::shared_ptr<SchedulerBase> ptr_t;
      typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      SchedulerBase(sdpa::daemon::IAgent* pHandler = NULL);
      virtual ~SchedulerBase();

      virtual void schedule(const sdpa::job_id_t&);
      virtual void schedule_local(const sdpa::job_id_t&);
      virtual void schedule_remotely(const sdpa::job_id_t&);
      void delete_job(const sdpa::job_id_t&);

      void schedule_first(const sdpa::job_id_t&);

      void rescheduleWorkerJob( const Worker::worker_id_t&, const sdpa::job_id_t&);
      void reschedule( const Worker::worker_id_t&, sdpa::job_id_list_t& );
      virtual bool has_job(const sdpa::job_id_t&);

      virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t&) throw (NoWorkerFoundException);
      virtual const Worker::ptr_t& findWorker(const Worker::worker_id_t&) throw(WorkerNotFoundException);
      virtual const Worker::worker_id_t& findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

      virtual void addWorker( const Worker::worker_id_t& workerId,
                              const boost::optional<unsigned int>& capacity = boost::none,
			      const capabilities_set_t& cpbset = capabilities_set_t(),
			      const unsigned int& agent_rank = 0,
			      const sdpa::worker_id_t& agent_uuid = "") throw (WorkerAlreadyExistException);

      virtual void deleteWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException);

      virtual void getWorkerList(sdpa::worker_id_list_t&);
      void getListNotFullWorkers(sdpa::worker_id_list_t& workerList);
      virtual Worker::worker_id_t getWorkerId(unsigned int rank);

      virtual void setLastTimeServed(const worker_id_t& wid, const sdpa::util::time_type& servTime);
      virtual size_t numberOfWorkers() { return ptr_worker_man_->numberOfWorkers(); }

      virtual bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      virtual void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException);
      virtual void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
      virtual void getWorkerCapabilities(const sdpa::worker_id_t&, sdpa::capabilities_set_t& cpbset);

      virtual void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException);

      sdpa::worker_id_t findSuitableWorker(const job_requirements_t&, sdpa::worker_id_list_t&);

      virtual void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException);

      virtual void feedWorkers();

      void set_timeout(long timeout) { m_timeout = boost::posix_time::microseconds(timeout); }

      // thread related functions
      virtual void start();
      virtual void stop();
      virtual void run();

      virtual void print();
      void printPendingJobs() { pending_jobs_queue_.print(); }

      bool schedulingAllowed() { return !ptr_worker_man_->common_queue_.empty(); }
      job_id_t nextJobToSchedule() { return ptr_worker_man_->common_queue_.pop(); }

    protected:
      JobQueue pending_jobs_queue_;
      WorkerManager::ptr_t ptr_worker_man_;

      bool bStopRequested;
      boost::thread m_thread_run;
      boost::thread m_thread_feed;

      sdpa::daemon::IAgent* ptr_comm_handler_;
      SDPA_DECLARE_LOGGER();
      boost::posix_time::time_duration m_timeout;

      mutable mutex_type mtx_;
      condition_type cond_feed_workers;
      condition_type cond_workers_registered;
    };
  }
}

BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::daemon::SchedulerBase )

#endif
