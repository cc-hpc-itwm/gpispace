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

      void store_result (worker_id_t const&, job_id_t const&, terminal_state);
      boost::optional<job_result_type>
        get_aggregated_results_if_all_terminated (job_id_t const&);

      // -- used by daemon and self
      void enqueueJob (const sdpa::job_id_t&);
      void request_scheduling();

      // used by daemon and self and test
      void releaseReservation (const sdpa::job_id_t&);
      void assignJobsToWorkers();
      void steal_work();
      assignment_t get_current_assignment_TESTING_ONLY() const;

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

      bool reservation_canceled (job_id_t const&) const;
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
        inline void push (job_id_t const& item);
        template <typename Range>
        inline void push (Range const& items);
        inline size_t erase (const job_id_t& item);

        std::list<job_id_t> get_and_clear();

      private:
        mutable boost::mutex mtx_;
        std::list<job_id_t> container_;
      } _jobs_to_schedule;

      class Reservation : boost::noncopyable
      {
      public:
        Reservation (std::set<worker_id_t> workers, double cost)
          : _workers (workers)
          , _cost (cost)
          , _canceled (false)
        {}

        void replace_worker (worker_id_t const& w1, worker_id_t w2)
        {
          if (!_workers.count (w1))
          {
            throw std::runtime_error ( "Asked to replace the non-existent worker " + w1);
          }

          _workers.erase (w1);
          _workers.emplace (std::move (w2));
        }

        std::set<worker_id_t> workers() const
        {
          return _workers;
        }

        double cost() const {return _cost;}

      private:
        std::set<worker_id_t> _workers;
        double _cost;

      public:
        //! \todo move to job statemachine instead: is duplicated
        //! state and is irrelevant to the scheduler. instances of
        //! this class should probably be deleted as soon as the
        //! workers are served, with the knowledge of where a job is
        //! served to being somewhere else (as it already has to be
        //! somewhere)

        void store_result
          (worker_id_t const& worker, terminal_state const& result)
        {
          if (!_results.individual_results.emplace (worker, result).second)
          {
            throw std::logic_error ("store_result: second result");
          }
          if ( JobFSM_::s_finished const* f
             = boost::get<JobFSM_::s_finished> (&result)
             )
          {
            _results.last_success = *f;
          }
        }

        boost::optional<job_result_type>
          get_aggregated_results_if_all_terminated() const
        {
          return boost::make_optional
            (_results.individual_results.size() == _workers.size(), _results);
        }

        bool apply_to_workers_without_result
          (std::function <void (worker_id_t const&)> fun) const
        {
          bool applied {false};

          for (worker_id_t const& worker_id : _workers)
          {
            if (!_results.individual_results.count (worker_id))
            {
              fun (worker_id);

              applied = true;
            }
          }

          return applied;
        }

        void cancel() {_canceled = true;}
        bool is_canceled() const {return _canceled;}

      private:
        job_result_type _results;
        bool _canceled;
      };

      mutable boost::mutex mtx_alloc_table_;
      typedef std::unordered_map<sdpa::job_id_t, Reservation*>
        allocation_table_t;
      allocation_table_t allocation_table_;
      locked_job_id_list _list_pending_jobs;
    };
  }
}
