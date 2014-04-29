// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_COALLOCSCHED_HPP
#define SDPA_COALLOCSCHED_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/job_requirements.hpp>
#include <sdpa/types.hpp>

#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/range/adaptor/map.hpp>

namespace sdpa
{
  namespace daemon
  {
    class CoallocationScheduler
    {
    public:
      CoallocationScheduler ( std::function<void (const sdpa::worker_id_list_t&, const job_id_t&)> serve
                            , std::function<job_requirements_t (const sdpa::job_id_t&)>
                            );

      const WorkerManager& worker_manager() const;
      WorkerManager& worker_manager();

      // -- used by daemon
      bool delete_job (const sdpa::job_id_t&);
      void workerFinished (const worker_id_t&, const job_id_t&);
      void workerFailed (const worker_id_t&, const job_id_t&);
      void workerCanceled (const worker_id_t&, const job_id_t&);
      bool allPartialResultsCollected (const job_id_t& jid);
      bool groupFinished (const sdpa::job_id_t&);
      worker_id_list_t workers (job_id_t) const;

      // -- used by daemon and self
      void enqueueJob (const sdpa::job_id_t&);
      void request_scheduling();

      // used by daemon and self and test
      void releaseReservation (const sdpa::job_id_t&);
      void assignJobsToWorkers();

      bool cancelNotTerminatedWorkerJobs ( std::function<void (worker_id_t const&)> func
                                         , const sdpa::job_id_t& job_id);

    private:
      std::function<void (const sdpa::worker_id_list_t&, const job_id_t&)>
        _serve_job;
      std::function<job_requirements_t (const sdpa::job_id_t&)>
        _job_requirements;

      WorkerManager _worker_manager;

      class locked_job_id_list
      {
      public:
        inline void push (job_id_t item);
        inline size_t erase (const job_id_t& item);

        std::list<job_id_t> get_and_clear();

      private:
        mutable boost::mutex mtx_;
        std::list<job_id_t> container_;
      } _jobs_to_schedule;

      class Reservation : boost::noncopyable
      {
      public:
        typedef enum {FINISHED, FAILED, CANCELED} result_type;

        Reservation (std::set<worker_id_t> workers)
          : m_list_workers (workers.begin(), workers.end())
        {}

        void storeWorkerResult
          (const sdpa::worker_id_t& wid, const result_type& result)
        {
          if ( std::find (m_list_workers.begin(), m_list_workers.end(), wid)
             == m_list_workers.end()
             )
          {
            throw std::runtime_error
              ("tried storing result of worker that is not in reservation for job");
          }

          m_map_worker_result[wid] = result;
        }

        bool allWorkersTerminated() const
        {
          return m_map_worker_result.size() == m_list_workers.size();
        }

        bool allGroupTasksFinishedSuccessfully()
        {
          for ( result_type result
              : m_map_worker_result | boost::adaptors::map_values
              )
          {
            if (result != FINISHED)
            {
              return false;
            }
          }
          return true;
        }

        sdpa::worker_id_list_t getWorkerList() const
        {
          return m_list_workers;
        }

        sdpa::worker_id_list_t getListNotTerminatedWorkers() const
        {
          sdpa::worker_id_list_t list_not_terminated_workers(m_list_workers);
          for ( const worker_id_t& wid
              : m_map_worker_result | boost::adaptors::map_keys
              )
          {
            list_not_terminated_workers.remove(wid);
          }

          return list_not_terminated_workers;
        }

      private:
        sdpa::worker_id_list_t m_list_workers;
        std::map<sdpa::worker_id_t, result_type> m_map_worker_result;
      };

      mutable boost::mutex mtx_alloc_table_;
      typedef std::unordered_map<sdpa::job_id_t, Reservation*>
        allocation_table_t;
      allocation_table_t allocation_table_;
    };
  }
}

#endif
