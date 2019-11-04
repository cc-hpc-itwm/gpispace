#pragma once

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/types.hpp>

#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <functional>
#include <mutex>

namespace sdpa
{
  namespace daemon
  {
    class CoallocationScheduler : boost::noncopyable
    {
    public:
      CoallocationScheduler
        ( std::function<Requirements_and_preferences (const sdpa::job_id_t&)>
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

      void reschedule_worker_jobs
        ( worker_id_t const&
        , std::function<Job* (sdpa::job_id_t const&)>
        , std::function<void (sdpa::worker_id_t const&, job_id_t const&)>
        , bool
        );

      std::set<job_id_t> start_pending_jobs
        (std::function<void ( std::set<Worker_and_implementation> const&
                            , const job_id_t&
                            )
                      >
        );

      bool reservation_canceled (job_id_t const&) const;
    private:
      double compute_reservation_cost
        ( const job_id_t&
        , const std::set<Worker_and_implementation>&
        , const double computational_cost
        ) const;

      std::function<Requirements_and_preferences (const sdpa::job_id_t&)>
        _requirements_and_preferences;

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
        mutable std::mutex mtx_;
        std::list<job_id_t> container_;
      } _jobs_to_schedule;

      class Reservation : boost::noncopyable
      {
      public:
        Reservation
            ( std::set<Worker_and_implementation> workers_and_implementations
            , double cost
            )
          : _workers_and_implementations (workers_and_implementations)
          , _cost (cost)
        {}

        void replace_worker
          ( worker_id_t const& w1
          , worker_id_t const& w2
          , std::function<bool (const std::string& cpb)> const&
              supports_implementation
          )
        {
          std::set<Worker_and_implementation>::iterator it
            (std::find_if
               ( _workers_and_implementations.begin()
               , _workers_and_implementations.end()
               , [&w1] (Worker_and_implementation const& x)
                 {
                   return x.worker() == w1;
                 }
               )
            );

          if (it == _workers_and_implementations.end())
          {
            throw std::runtime_error
              ("Asked to replace the non-existent worker " + w1);
          }

          auto const& implementation (it->implementation());
          if (implementation && !supports_implementation (*implementation))
          {
            throw std::runtime_error
              ( ( boost::format
                    ( "Cannot replace worker %1% with worker %2%: "
                      "%3% does not support the implementation %4%."
                    )
                % w1
                % w2
                % w2
                % *implementation
                ).str()
              );
          }

          _workers_and_implementations.emplace (w2, it->implementation());
          _workers_and_implementations.erase (it);
        }

        std::set<worker_id_t> workers() const
        {
          std::set<worker_id_t> workers;
          std::transform ( _workers_and_implementations.begin()
                         , _workers_and_implementations.end()
                         , std::inserter (workers, workers.begin())
                         , [] (Worker_and_implementation const& worker_impl)
                           {
                             return worker_impl.worker();
                           }
                         );

          return workers;
        }

        std::set<Worker_and_implementation> workers_and_implementations() const
        {
          return _workers_and_implementations;
        }

        double cost() const {return _cost;}

      private:
        std::set<Worker_and_implementation> _workers_and_implementations;
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
        void mark_as_canceled_if_no_result_stored_yet (worker_id_t const& worker)
        {
          _results.individual_results.emplace (worker, JobFSM_::s_canceled());
        }

        boost::optional<job_result_type>
          get_aggregated_results_if_all_terminated() const
        {
          return boost::make_optional
            ( _results.individual_results.size() == _workers_and_implementations.size()
            , _results
            );
        }

        bool apply_to_workers_without_result
          (std::function <void (worker_id_t const&)> fun) const
        {
          bool applied {false};

          for (auto const& worker_and_impl : _workers_and_implementations)
          {
            if (!_results.individual_results.count (worker_and_impl.worker()))
            {
              fun (worker_and_impl.worker());

              applied = true;
            }
          }

          return applied;
        }

        bool is_canceled() const
        {
          return std::any_of
            ( _results.individual_results.begin()
            , _results.individual_results.end()
            , [] (std::pair<sdpa::worker_id_t, terminal_state> const& result)
              {
                return boost::get<JobFSM_::s_canceled> (&result.second);
              }
            );
        }

      private:
        job_result_type _results;
      };

      //! \note to be able to call releaseReservation instead of
      //! reimplementing in reschedule_worker_jobs
      mutable std::recursive_mutex mtx_alloc_table_;
      typedef std::unordered_map<sdpa::job_id_t, Reservation*>
        allocation_table_t;
      allocation_table_t allocation_table_;
      std::unordered_set<job_id_t> _pending_jobs;

      friend class access_allocation_table_TESTING_ONLY;
    };
  }
}
