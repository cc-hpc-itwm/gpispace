// tiberiu.rotaru@itwm.fraunhofer.de

#pragma once

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
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
      typedef std::map<job_id_t, std::set<worker_id_t>> assignment_t;

      CoallocationScheduler
        ( std::function<job_requirements_t (const sdpa::job_id_t&)>
        , WorkerManager&
        );

      // -- used by daemon
      bool delete_job (const sdpa::job_id_t&);
      void workerFinished (const worker_id_t&, const job_id_t&);
      void workerFailed (const worker_id_t&, const job_id_t&);
      void workerCanceled (const worker_id_t&, const job_id_t&);
      bool allPartialResultsCollected (const job_id_t& jid);
      bool groupFinished (const sdpa::job_id_t&);

      // -- used by daemon and self
      void enqueueJob (const sdpa::job_id_t&);
      void request_scheduling();

      // used by daemon and self and test
      void releaseReservation (const sdpa::job_id_t&);
      assignment_t assignJobsToWorkers();

      bool cancelNotTerminatedWorkerJobs ( std::function<void (worker_id_t const&)> func
                                         , const sdpa::job_id_t& job_id);

      std::set<worker_id_t> find_job_assignment_minimizing_total_cost
        ( const mmap_match_deg_worker_id_t& mmap_matching_workers
        , const size_t n_req_workers
        , const std::function<double (std::string const&)> transfer_cost
        , const double computational_cost
        );

      void reschedule_pending_jobs_matching_worker (const worker_id_t&);
      std::set<job_id_t> start_pending_jobs
        (std::function<void (std::set<worker_id_t> const&, const job_id_t&)>);
    private:
      double compute_reservation_cost
        ( const job_id_t&
        , const std::set<worker_id_t>&
        , const double computational_cost
        ) const;

      std::function<job_requirements_t (const sdpa::job_id_t&)>
        _job_requirements;

      WorkerManager& _worker_manager;

      class locked_job_id_list
      {
      public:
        inline void push (job_id_t item);
        template <typename Range>
        inline void push (Range items);
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

        Reservation (std::set<worker_id_t> workers, double cost)
          : _workers (workers)
          , _cost (cost)
        {}

        void storeWorkerResult
          (const sdpa::worker_id_t& wid, const result_type& result)
        {
          if (_workers.count (wid) == 0)
          {
            throw std::runtime_error
              ("tried storing the result of a worker that doesn't exist in the job reservation");
          }

          m_map_worker_result[wid] = result;
        }

        void replace_worker (worker_id_t const& w1, worker_id_t w2)
        {
          if (!_workers.count (w1))
          {
            throw std::runtime_error ( "Asked to replace the non-existent worker " + w1);
          }

          _workers.erase (w1);
          _workers.emplace (std::move (w2));
        }

        bool allWorkersTerminated() const
        {
          return m_map_worker_result.size() == _workers.size();
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

        std::set<worker_id_t> workers() const
        {
          return _workers;
        }

        sdpa::worker_id_list_t getListNotTerminatedWorkers() const
        {
          sdpa::worker_id_list_t list_not_terminated_workers (_workers.begin(), _workers.end());
          for ( const worker_id_t& wid
              : m_map_worker_result | boost::adaptors::map_keys
              )
          {
            list_not_terminated_workers.remove(wid);
          }

          return list_not_terminated_workers;
        }

        double cost() const {return _cost;}
      private:
        std::set<worker_id_t> _workers;
        std::map<sdpa::worker_id_t, result_type> m_map_worker_result;
        double _cost;
      };

      mutable boost::mutex mtx_alloc_table_;
      typedef std::unordered_map<sdpa::job_id_t, Reservation*>
        allocation_table_t;
      allocation_table_t allocation_table_;
      locked_job_id_list _list_pending_jobs;
    };
  }
}
