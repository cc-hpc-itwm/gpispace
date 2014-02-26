// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_COALLOCSCHED_HPP
#define SDPA_COALLOCSCHED_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/types.hpp>

#include <boost/optional.hpp>
#include <boost/thread.hpp>

namespace sdpa {
  namespace daemon {
    class Reservation : boost::noncopyable
    {
    public:
      typedef enum {FINISHED, FAILED, CANCELED} result_type;
      typedef std::map<sdpa::worker_id_t, result_type> map_worker_result_t;

      Reservation(const sdpa::job_id_t& job_id, const size_t& n) : m_job_id(job_id), m_capacity(n) {}

      size_t size() const { return m_list_workers.size(); }
      void addWorker(const sdpa::worker_id_t& wid) { m_list_workers.push_back(wid); }
      void delWorker(const sdpa::worker_id_t& wid) { m_list_workers.remove(wid); }

      void storeWorkerResult(const sdpa::worker_id_t& wid, const result_type& result)
      {
        if(!hasWorker(wid))
        {
            std::string errMsg("No worker ");
            errMsg+=wid;
            errMsg+=" exists within the current reservation of the job ";
            errMsg+=m_job_id;
            throw std::runtime_error(errMsg);
        }
        else {
          // take the rseult and store it!
          map_worker_result_t::iterator it = m_map_worker_result.find(wid);
          if( it != m_map_worker_result.end() ) {
              it->second = result;
          } else {
              m_map_worker_result.insert(map_worker_result_t::value_type(wid, result));
          }
        }
      }

      void workerFinished(const sdpa::worker_id_t& wid) {
        storeWorkerResult(wid, FINISHED);
      }

      void workerFailed(const sdpa::worker_id_t& wid) {
        storeWorkerResult(wid, FAILED);
      }

      void workerCanceled(const sdpa::worker_id_t& wid) {
        storeWorkerResult(wid, CANCELED);
      }

      // should protect this!!!!
      bool allWorkersTerminated() const {return m_map_worker_result.size() == capacity(); }

      bool allGroupTasksFinishedSuccessfully()
      {
        for(map_worker_result_t::iterator it(m_map_worker_result.begin()); it!=m_map_worker_result.end(); it++)
          if(it->second!=FINISHED)
            return false;
        return true;
      }

      bool acquired() const { return (size()==capacity()); }

      bool hasWorker(const sdpa::worker_id_t& wid) const { return find(m_list_workers.begin(), m_list_workers.end(), wid)!=m_list_workers.end(); }
      sdpa::worker_id_list_t getWorkerList() const { return m_list_workers; }

    private:
      size_t capacity() const { return m_capacity; }

      sdpa::job_id_t m_job_id;
      size_t m_capacity;
      sdpa::worker_id_list_t m_list_workers;
      map_worker_result_t m_map_worker_result;
    };

    class GenericDaemon;
    class CoallocationScheduler
    {
    public:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      CoallocationScheduler(GenericDaemon* pHandler, bool TEST_without_threads = false);
      ~CoallocationScheduler();

      // -- used by daemon
      void delete_job(const sdpa::job_id_t&);
      const boost::optional<Worker::worker_id_t> findSubmOrAckWorker(const sdpa::job_id_t& job_id) const;
      void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
      sdpa::capabilities_set_t getWorkerCapabilities(const sdpa::worker_id_t&);
      void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id);
      void workerFinished(const worker_id_t& wid, const job_id_t& jid);
      void workerFailed(const worker_id_t& wid, const job_id_t& jid);
      void workerCanceled(const worker_id_t& wid, const job_id_t& jid);
      bool allPartialResultsCollected(const job_id_t& jid);
      bool groupFinished(const sdpa::job_id_t& jid);

      // -- used by daemon and self
      void rescheduleWorkerJob( const Worker::worker_id_t&, const sdpa::job_id_t&);
      void enqueueJob(const sdpa::job_id_t&);
      Worker::ptr_t findWorker(const Worker::worker_id_t&);

      // used by daemon and test
      bool addWorker( const Worker::worker_id_t& workerId,const boost::optional<unsigned int>& capacity = boost::none,const capabilities_set_t& cpbset = capabilities_set_t());
      void deleteWorker( const Worker::worker_id_t& workerId);
      bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);

      // used by daemon and self and test
      void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id );
      void releaseReservation(const sdpa::job_id_t& jobId);

      // -- used by self and test
      void schedule(const sdpa::job_id_t&);
      void assignJobsToWorkers();

    private:
      void feedWorkers();
      void run();


      GenericDaemon* ptr_comm_handler_;
      fhg::log::Logger::ptr_t _logger;

      SynchronizedQueue<std::list<sdpa::job_id_t> >  pending_jobs_queue_;
      WorkerManager _worker_manager;

      mutable mutex_type mtx_;
      boost::condition_variable_any cond_feed_workers;
      boost::condition_variable_any cond_workers_registered;

      boost::thread m_thread_run;
      boost::thread m_thread_feed;

      mutable mutex_type mtx_alloc_table_;
      typedef boost::unordered_map<sdpa::job_id_t, Reservation*> allocation_table_t;
      allocation_table_t allocation_table_;
  };
}}

#endif
