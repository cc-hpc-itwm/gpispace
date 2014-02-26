// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_COALLOCSCHED_HPP
#define SDPA_COALLOCSCHED_HPP 1

#include <boost/thread.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>

#include <boost/optional.hpp>
#include <sdpa/daemon/WorkerManager.hpp>

#include <boost/optional.hpp>

namespace sdpa {
  namespace daemon {
    class GenericDaemon;
    class CoallocationScheduler
    {
    public:
      typedef boost::shared_ptr<CoallocationScheduler> ptr_t;
      typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      CoallocationScheduler(GenericDaemon* pHandler, bool TEST_without_threads = false);
      ~CoallocationScheduler();

      void enqueueJob(const sdpa::job_id_t&);
      void schedule(const sdpa::job_id_t&);
      void schedule(Job*);
      void delete_job(const sdpa::job_id_t&);
      void assignJobsToWorkers();

      void schedule_first(const sdpa::job_id_t&);

      void rescheduleWorkerJob( const Worker::worker_id_t&, const sdpa::job_id_t&);
      void rescheduleJob(const sdpa::job_id_t&);
      void reschedule( const Worker::worker_id_t&, sdpa::job_id_list_t& );
      bool has_job(const sdpa::job_id_t&);

      bool hasWorker(const Worker::worker_id_t&) const;
      Worker::ptr_t findWorker(const Worker::worker_id_t&);
      const boost::optional<Worker::worker_id_t> findSubmOrAckWorker(const sdpa::job_id_t& job_id) const;

      bool addWorker( const Worker::worker_id_t& workerId,
                      const boost::optional<unsigned int>& capacity = boost::none,
                      const capabilities_set_t& cpbset = capabilities_set_t());

      void deleteWorker( const Worker::worker_id_t& workerId);

      void getListNotFullWorkers(sdpa::worker_id_list_t& workerList);

      size_t numberOfWorkers() { return _worker_manager.numberOfWorkers(); }

      bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
      sdpa::capabilities_set_t getWorkerCapabilities(const sdpa::worker_id_t&);

      void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id );

      boost::optional<sdpa::worker_id_t> findSuitableWorker(const job_requirements_t&, const sdpa::worker_id_list_t&);

      void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id);

      bool schedulingAllowed() { return !_worker_manager.common_queue_.empty(); }
      job_id_t nextJobToSchedule() { return _worker_manager.common_queue_.pop(); }

    private:
      void feedWorkers();
      void run();

    protected:
      GenericDaemon* ptr_comm_handler_;
      fhg::log::Logger::ptr_t _logger;

      JobQueue pending_jobs_queue_;
      WorkerManager _worker_manager;

      mutable mutex_type mtx_;
      condition_type cond_feed_workers;
      condition_type cond_workers_registered;

      boost::thread m_thread_run;
      boost::thread m_thread_feed;

    public:
      typedef boost::unordered_map<sdpa::job_id_t, Reservation*> allocation_table_t;

      void reserveWorker(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& matchingWorkerId, const size_t& cap);
      void releaseReservation(const sdpa::job_id_t& jobId);

      sdpa::worker_id_list_t getListAllocatedWorkers(const sdpa::job_id_t& jobId);

      void workerFinished(const worker_id_t& wid, const job_id_t& jid);
      void workerFailed(const worker_id_t& wid, const job_id_t& jid);
      void workerCanceled(const worker_id_t& wid, const job_id_t& jid);
      bool allPartialResultsCollected(const job_id_t& jid);
      bool groupFinished(const sdpa::job_id_t& jid);

      sdpa::worker_id_list_t checkReservationIsValid(const Reservation& res);
    private:
      mutable mutex_type mtx_alloc_table_;
      allocation_table_t allocation_table_;
  };
}}

#endif
