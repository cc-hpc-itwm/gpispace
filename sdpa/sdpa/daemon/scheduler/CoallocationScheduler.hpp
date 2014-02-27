// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_COALLOCSCHED_HPP
#define SDPA_COALLOCSCHED_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/types.hpp>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

namespace sdpa
{
  namespace daemon
  {
    class GenericDaemon;
    class CoallocationScheduler
    {
    public:
      CoallocationScheduler
        (GenericDaemon* pHandler, bool TEST_without_threads = false);
      ~CoallocationScheduler();

      // -- used by daemon
      void delete_job (const sdpa::job_id_t&);
      const boost::optional<Worker::worker_id_t> findSubmOrAckWorker
        (const sdpa::job_id_t&) const;
      void removeCapabilities
        (const sdpa::worker_id_t&, const sdpa::capabilities_set_t&);
      void getAllWorkersCapabilities (sdpa::capabilities_set_t&);
      sdpa::capabilities_set_t getWorkerCapabilities (const sdpa::worker_id_t&);
      void acknowledgeJob (const Worker::worker_id_t&, const sdpa::job_id_t&);
      void workerFinished (const worker_id_t&, const job_id_t&);
      void workerFailed (const worker_id_t&, const job_id_t&);
      void workerCanceled (const worker_id_t&, const job_id_t&);
      bool allPartialResultsCollected (const job_id_t& jid);
      bool groupFinished (const sdpa::job_id_t&);

      // -- used by daemon and self
      void rescheduleWorkerJob
        (const Worker::worker_id_t&, const sdpa::job_id_t&);
      void enqueueJob (const sdpa::job_id_t&);
      Worker::ptr_t findWorker (const Worker::worker_id_t&);

      // used by daemon and test
      bool addWorker ( const Worker::worker_id_t&
                     , const boost::optional<unsigned int>& capacity = boost::none
                     , const capabilities_set_t& = capabilities_set_t()
                     );
      void deleteWorker (const Worker::worker_id_t&);
      bool addCapabilities
        (const sdpa::worker_id_t&, const sdpa::capabilities_set_t&);

      // used by daemon and self and test
      void deleteWorkerJob (const Worker::worker_id_t&, const sdpa::job_id_t&);
      void releaseReservation (const sdpa::job_id_t&);

      // -- used by self and test
      void assignJobsToWorkers();

    private:
      void feedWorkers();


      GenericDaemon* ptr_comm_handler_;

      SynchronizedQueue<std::list<sdpa::job_id_t> > pending_jobs_queue_;
      WorkerManager _worker_manager;
      SynchronizedQueue<std::list<sdpa::job_id_t> > _common_queue;

      mutable boost::recursive_mutex mtx_;
      boost::condition_variable_any cond_feed_workers;

      boost::thread m_thread_feed;

      class Reservation : boost::noncopyable
      {
      public:
        typedef enum {FINISHED, FAILED, CANCELED} result_type;

        Reservation (const sdpa::job_id_t& job_id, const size_t& n)
          : m_job_id (job_id)
          , m_capacity (n)
        {}

        void addWorker (const sdpa::worker_id_t& wid)
        {
          m_list_workers.push_back(wid);
        }
        void delWorker (const sdpa::worker_id_t& wid)
        {
          m_list_workers.remove(wid);
        }

        void storeWorkerResult
          (const sdpa::worker_id_t& wid, const result_type& result)
        {
          if (!hasWorker (wid))
          {
            throw std::runtime_error
              ("tried storing result of worker that is not in reservation for job");
          }

          m_map_worker_result[wid] = result;
        }

        bool allWorkersTerminated() const
        {
          return m_map_worker_result.size() == capacity();
        }

        bool allGroupTasksFinishedSuccessfully()
        {
          BOOST_FOREACH ( result_type result
                        , m_map_worker_result | boost::adaptors::map_values
                        )
          {
            if (result != FINISHED)
            {
              return false;
            }
          }
          return true;
        }

        bool acquired() const
        {
          return size() == capacity();
        }

        bool hasWorker (const sdpa::worker_id_t& wid) const
        {
          return find (m_list_workers.begin(), m_list_workers.end(), wid)
            != m_list_workers.end();
        }
        sdpa::worker_id_list_t getWorkerList() const
        {
          return m_list_workers;
        }

      private:
        size_t size() const
        {
          return m_list_workers.size();
        }
        size_t capacity() const
        {
          return m_capacity;
        }

        sdpa::job_id_t m_job_id;
        size_t m_capacity;
        sdpa::worker_id_list_t m_list_workers;
        std::map<sdpa::worker_id_t, result_type> m_map_worker_result;
      };

      mutable boost::mutex mtx_alloc_table_;
      typedef boost::unordered_map<sdpa::job_id_t, Reservation*>
        allocation_table_t;
      allocation_table_t allocation_table_;
    };
  }
}

#endif
