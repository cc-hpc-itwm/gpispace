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

namespace sdpa {
  namespace daemon {
    class SchedulerImpl : public Scheduler
    {
    public:
      typedef sdpa::shared_ptr<SchedulerImpl> ptr_t;
      typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
      typedef boost::unordered_map<sdpa::job_id_t, sdpa::worker_id_list_t> allocation_table_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      SchedulerImpl(sdpa::daemon::IAgent* pHandler = NULL);
      virtual ~SchedulerImpl();

      virtual void schedule(const sdpa::job_id_t&);
      virtual void schedule_local(const sdpa::job_id_t&);
      virtual void schedule_remotely(const sdpa::job_id_t&);
      void delete_job(const sdpa::job_id_t&);

      bool schedule_to( const sdpa::job_id_t&, const sdpa::worker_id_t& );
      bool schedule_to( const sdpa::job_id_t&, const Worker::ptr_t& pWorker );

      void reschedule( const sdpa::job_id_t& );
      void reschedule( const Worker::worker_id_t&);
      void reschedule( const Worker::worker_id_t&, const sdpa::job_id_t&);

      virtual bool has_job(const sdpa::job_id_t&);

      virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t&) throw (NoWorkerFoundException);
      virtual const Worker::ptr_t& findWorker(const Worker::worker_id_t&) throw(WorkerNotFoundException);
      virtual const Worker::worker_id_t& findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

      virtual void addWorker( const Worker::worker_id_t& workerId,
                              const unsigned int& capacity = 10000,
			      const capabilities_set_t& cpbset = capabilities_set_t(),
			      const unsigned int& agent_rank = 0,
			      const sdpa::worker_id_t& agent_uuid = "") throw (WorkerAlreadyExistException);

      virtual void deleteWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException);
      void declare_jobs_failed( const Worker::worker_id_t&, Worker::JobQueue* pQueue );

      virtual void getWorkerList(sdpa::worker_id_list_t&);
      void getListNotFullWorkers(sdpa::worker_id_list_t& workerList);
      void getListNotAllocatedWorkers(sdpa::worker_id_list_t& workerList);
      virtual Worker::worker_id_t getWorkerId(unsigned int rank);
      sdpa::job_id_t getAssignedJob(const sdpa::worker_id_t&);
      sdpa::worker_id_list_t getListAllocatedWorkers(const sdpa::job_id_t& jobId) { return allocation_table_[jobId]; }

      //! \todo Eliminate that function, is used in a test only
      sdpa::job_id_t getNextJobToSchedule();

      void assignJobsToWorkers();

      virtual void setLastTimeServed(const worker_id_t& wid, const sdpa::util::time_type& servTime);
      virtual size_t numberOfWorkers() { return ptr_worker_man_->numberOfWorkers(); }

      virtual bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      virtual void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException);
      virtual void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
      virtual void getWorkerCapabilities(const sdpa::worker_id_t&, sdpa::capabilities_set_t& cpbset);

      virtual void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException);

      sdpa::worker_id_t findSuitableWorker(const job_requirements_t&, sdpa::worker_id_list_t&);
      void releaseAllocatedWorkers(const sdpa::job_id_t& jobId);
      void reserveWorker(const sdpa::job_id_t&, const sdpa::worker_id_t&) throw (WorkerReservationFailed);

      virtual void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException);
      virtual void execute(const sdpa::job_id_t& jobId); //just for testing

      virtual void feedWorkers();
      void cancelWorkerJobs();
      void planForCancellation(const Worker::worker_id_t& workerId, const sdpa::job_id_t& jobId);
      virtual void forceOldWorkerJobsTermination();

      void set_timeout(long timeout) { m_timeout = boost::posix_time::microseconds(timeout); }

      // thread related functions
      virtual void start(IAgent*);
      virtual void stop();
      virtual void run();

      virtual void print();
      void removeWorkers() { ptr_worker_man_->removeWorkers(); }

      void printPendingJobs() { pending_jobs_queue_.print(); }
      void printAllocationTable();
      void checkAllocations();

    protected:
      JobQueue pending_jobs_queue_;
      WorkerManager::ptr_t ptr_worker_man_;

      bool bStopRequested;
      boost::thread m_thread_run;
      boost::thread m_thread_feed;

      mutable sdpa::daemon::IAgent* ptr_comm_handler_;
      SDPA_DECLARE_LOGGER();
      boost::posix_time::time_duration m_timeout;

      sdpa::cancellation_list_t cancellation_list_;

      mutable mutex_type mtx_;
      condition_type cond_feed_workers;
      condition_type cond_workers_registered;

      mutable mutex_type mtx_alloc_table_;
      allocation_table_t allocation_table_;

    };
  }
}

#endif
