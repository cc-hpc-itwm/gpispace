// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_COALLOCSCHED_HPP
#define SDPA_COALLOCSCHED_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

namespace sdpa {
  namespace daemon {
    class GenericDaemon;
    class CoallocationScheduler
    {
    public:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      CoallocationScheduler(GenericDaemon* pHandler, bool TEST_without_threads = false);
      ~CoallocationScheduler();

      void enqueueJob(const sdpa::job_id_t&);
      void schedule(const sdpa::job_id_t&);
      void delete_job(const sdpa::job_id_t&);
      void assignJobsToWorkers();

      void schedule_first(const sdpa::job_id_t&);

      void rescheduleWorkerJob( const Worker::worker_id_t&, const sdpa::job_id_t&);
      void reschedule( const Worker::worker_id_t&, sdpa::job_id_list_t& );
      bool has_job(const sdpa::job_id_t&);

      bool hasWorker(const Worker::worker_id_t&) const;
      Worker::ptr_t findWorker(const Worker::worker_id_t&);
      const boost::optional<Worker::worker_id_t> findSubmOrAckWorker(const sdpa::job_id_t& job_id) const;

      bool addWorker( const Worker::worker_id_t& workerId,
                      const boost::optional<unsigned int>& capacity = boost::none,
                      const capabilities_set_t& cpbset = capabilities_set_t());

      void deleteWorker( const Worker::worker_id_t& workerId);

      size_t numberOfWorkers() { return _worker_manager.numberOfWorkers(); }

      bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
      sdpa::capabilities_set_t getWorkerCapabilities(const sdpa::worker_id_t&);

      void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id );

      boost::optional<sdpa::worker_id_t> findSuitableWorker(const job_requirements_t&, const sdpa::worker_id_list_t&);

      void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id);

    private:
      void feedWorkers();
      void run();

    protected:
      GenericDaemon* ptr_comm_handler_;
      fhg::log::Logger::ptr_t _logger;

      SynchronizedQueue<std::list<sdpa::job_id_t> >  pending_jobs_queue_;
      WorkerManager _worker_manager;

      mutable mutex_type mtx_;
      boost::condition_variable_any cond_feed_workers;
      boost::condition_variable_any cond_workers_registered;

      boost::thread m_thread_run;
      boost::thread m_thread_feed;

    public:
      typedef boost::unordered_map<sdpa::job_id_t, Reservation*> allocation_table_t;

      void reserveWorker(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& matchingWorkerId, const size_t& cap);
      void releaseReservation(const sdpa::job_id_t& jobId);

      void workerFinished(const worker_id_t& wid, const job_id_t& jid);
      void workerFailed(const worker_id_t& wid, const job_id_t& jid);
      void workerCanceled(const worker_id_t& wid, const job_id_t& jid);
      bool allPartialResultsCollected(const job_id_t& jid);
      bool groupFinished(const sdpa::job_id_t& jid);

    private:
      mutable mutex_type mtx_alloc_table_;
      allocation_table_t allocation_table_;
  };
}}

#endif
