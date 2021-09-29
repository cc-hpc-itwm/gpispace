// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <fhgcom/address.hpp>

#include <fhg/util/boost/optional.hpp>

#include <boost/range/algorithm.hpp>

#include <algorithm>
#include <climits>
#include <functional>
#include <iterator>
#include <list>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace sdpa
{
  namespace daemon
  {
    namespace
    {
      struct cost_and_matching_info_t
      {
        cost_and_matching_info_t
            ( double cost
            , double matching_degree
            , unsigned long shared_memory_size
            , double last_time_idle
            , worker_id_t const& worker_id
            , boost::optional<std::string>const& implementation
            , double transfer_cost
            )
          : _cost (cost)
          , _matching_degree (matching_degree)
          , _shared_memory_size (shared_memory_size)
          , _last_time_idle (last_time_idle)
          , _worker_id (worker_id)
          , _implementation (implementation)
          , _transfer_cost (transfer_cost)
        {}

        double _cost;
        double _matching_degree;
        unsigned long _shared_memory_size;
        double _last_time_idle;
        worker_id_t _worker_id;
        boost::optional<std::string> _implementation;
        double _transfer_cost;
      };

      class Compare
       {
       public:
         bool operator() (cost_and_matching_info_t const& l, cost_and_matching_info_t const& r)
         {
           return std::tie ( l._cost, l._matching_degree
                           , l._shared_memory_size, l._last_time_idle
                           )
             < std::tie ( r._cost, r._matching_degree
                        , l._shared_memory_size,  r._last_time_idle
                        );
         }
       };

      typedef std::priority_queue < cost_and_matching_info_t
                                  , std::vector<cost_and_matching_info_t>
                                  , Compare
                                  > base_priority_queue_t;

      class bounded_priority_queue_t : private base_priority_queue_t
      {
      public:
        explicit bounded_priority_queue_t (std::size_t capacity)
          : capacity_ (capacity)
        {}

        template<typename... Args>
          void emplace (Args&&... args)
        {
          if (size() < capacity_)
          {
            base_priority_queue_t::emplace (std::forward<Args> (args)...);
            return;
          }

          cost_and_matching_info_t const next_tuple (std::forward<Args> (args)...);

          if (comp (next_tuple, top()))
          {
            pop();
            base_priority_queue_t::emplace (std::move (next_tuple));
          }
        }

        Workers_implementation_and_transfer_cost
          assigned_workers_and_implementation() const
        {
          WorkerSet workers;
          auto implementation (c.front()._implementation);
          double total_transfer_cost (0.0);

          std::transform ( c.begin()
                         , c.end()
                         , std::inserter (workers, workers.begin())
                         , [&total_transfer_cost]
                           (cost_and_matching_info_t const& cost_and_matching_info)
                           {
                             total_transfer_cost += cost_and_matching_info._transfer_cost;
                             return cost_and_matching_info._worker_id;
                           }
                         );


          return std::make_tuple<WorkerSet, Implementation, double>
            (std::move (workers), std::move (implementation), std::move (total_transfer_cost));
        }

        std::size_t size() const { return base_priority_queue_t::size(); }
      private:
        size_t capacity_;
      };
    }

    CoallocationScheduler::CoallocationScheduler
        (std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
           requirements_and_preferences
        )
      : _requirements_and_preferences (requirements_and_preferences)
      , _worker_manager()
    {}

    void CoallocationScheduler::delete_job (sdpa::job_id_t const& job)
    {
      _pending_jobs.erase (job);
      _jobs_to_schedule.erase (job);
    }

    void CoallocationScheduler::submit_job (sdpa::job_id_t const& jobId)
    {
      _jobs_to_schedule.push (jobId);
    }

    void CoallocationScheduler::assign_jobs_to_workers()
    {
      std::list<job_id_t> jobs_to_schedule (_jobs_to_schedule.get_and_clear());
      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t const jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const Requirements_and_preferences requirements_and_preferences
          (_requirements_and_preferences (jobId));

        if ( !requirements_and_preferences.preferences().empty()
           && requirements_and_preferences.numWorkers() > 1
           )
        {
          throw std::runtime_error
            ("Coallocation with preferences is forbidden!");
        }

        std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
        std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);
        WorkerSet workers;
        Implementation implementation;
        double total_transfer_cost;

        std::tie (workers, implementation, total_transfer_cost)
          = find_assignment (requirements_and_preferences, lock_worker_man);

        if (!workers.empty())
        {
          if (allocation_table_.find (jobId) != allocation_table_.end())
          {
            throw std::runtime_error ("already have reservation for job " + jobId);
          }

          double cost
            ( total_transfer_cost
            + requirements_and_preferences.computational_cost()
            );

          try
          {
            for (auto const& worker : workers)
            {
              _worker_manager.assign_job_to_worker
                ( jobId
                , worker
                , cost
                , requirements_and_preferences.preferences()
                );
            }

            allocation_table_.emplace
              ( jobId
              , std::make_unique<scheduler::Reservation>
                  ( workers
                  , implementation
                  , requirements_and_preferences.preferences()
                  , cost
                  )
              );

            _pending_jobs.emplace (jobId);
          }
          catch (std::out_of_range const&)
          {
            for (auto const&  worker : workers)
            {
              _worker_manager.delete_job_from_worker (jobId, worker, cost);
            }

            jobs_to_schedule.push_front (jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      _jobs_to_schedule.push (jobs_to_schedule);
      _jobs_to_schedule.push (nonmatching_jobs_queue);
    }

    void CoallocationScheduler::steal_work()
    {
      std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      _worker_manager.steal_work
        ( [this] (job_id_t const& job)
          {
            return allocation_table_.at (job).get();
          }
        );
    }

    void CoallocationScheduler::reschedule_worker_jobs_and_maybe_remove_worker
       ( fhg::com::p2p::address_t const& source
       , std::function<Job* (sdpa::job_id_t const&)> get_job
       , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)> cancel_worker_job
       )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      auto const as_worker
        (_worker_manager.worker_by_address (source));

      if (as_worker)
      {
        auto const worker (as_worker.get()->second);
        for ( job_id_t const& job_id
            : delete_or_cancel_worker_jobs
                ( worker
                , get_job
                , cancel_worker_job
                , lock_alloc_table
                , lock_worker_man
                )
            )
        {
          release_reservation (job_id, lock_alloc_table, lock_worker_man);
          delete_job (job_id);
          submit_job (job_id);
        }

        _worker_manager.delete_worker (worker);
      }
    }

    void CoallocationScheduler::start_pending_jobs
      ( std::function<void ( WorkerSet const&
                           , Implementation const& implementation
                           , job_id_t const&
                           , std::function<fhg::com::p2p::address_t (worker_id_t const&)>
                           )
                     > serve_job
      )
    {
      std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      for ( auto it (_pending_jobs.begin())
          ; _worker_manager.num_free_workers() > 0
          && it != _pending_jobs.end()
          ;
          )
      {
        auto const job_id (*it);

        auto const job_reservation (allocation_table_.at (job_id).get());
        auto const workers (job_reservation->workers());
        auto const implementation (job_reservation->implementation());

        if (_worker_manager.all_free (workers))
        {
          for (auto const& worker: workers)
          {
            _worker_manager.submit_job_to_worker (job_id, worker);
          }

          serve_job
            ( workers
            , implementation
            , job_id
            , [this] (worker_id_t const& worker)
              {
                return _worker_manager.address_by_worker (worker).get()->second;
              }
            );

          it = _pending_jobs.erase (it);
        }
        else
        {
          it++;
        }
      }
    }

    void CoallocationScheduler::release_reservation (sdpa::job_id_t const& job_id)
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      release_reservation (job_id, lock_alloc_table, lock_worker_man);
    }

    void CoallocationScheduler::release_reservation
      ( sdpa::job_id_t const& job_id
      , std::lock_guard<std::mutex> const&
      , std::lock_guard<std::mutex> const&
      )
    {
      auto const it (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        for (std::string const& worker : it->second->workers())
        {
          try
          {
            _worker_manager.delete_job_from_worker
              (job_id, worker, it->second->cost());
          }
          catch (...)
          {
            //! \note can be ignored: was deleted using delete_worker()
            //! which correctly clears queues already, and
            //! delete_job_from_worker does nothing else.
          }
        }

        allocation_table_.erase (it);
      }
      //! \todo why can we ignore this?
    }

    bool CoallocationScheduler::reschedule_job_if_the_reservation_was_canceled
      (job_id_t const& job, status::code const status)
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      if ( !allocation_table_.at (job)->is_canceled()
        || (status == sdpa::status::CANCELING)
         )
      {
        return false;
      }

      release_reservation (job, lock_alloc_table, lock_worker_man);
      submit_job (job);

      return true;
    }

    void CoallocationScheduler::store_result
      ( fhg::com::p2p::address_t const& worker_addr
      , job_id_t const& job_id
      , terminal_state result
      )
    {
      std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error ("store_result: unknown job");
      }

      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      auto const worker
        ( fhg::util::boost::get_or_throw<std::invalid_argument>
            ( _worker_manager.worker_by_address (worker_addr)
            , "attempting to store job result from unknown worker!"
            )
        );

      it->second->store_result (worker->second, result);
    }

    boost::optional<job_result_type>
      CoallocationScheduler::get_aggregated_results_if_all_terminated (job_id_t const& job_id)
    {
      std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error
          ("get_aggregated_results_if_all_terminated: unknown job");
      }

      return it->second->get_aggregated_results_if_all_terminated();
    }

    void CoallocationScheduler::locked_job_id_list::push (job_id_t const& item)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      container_.emplace_back (item);
    }

    template <typename Range>
    void CoallocationScheduler::locked_job_id_list::push (Range const& range)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      container_.insert (container_.end(), std::begin (range), std::end (range));
    }

    size_t CoallocationScheduler::locked_job_id_list::erase (job_id_t const& item)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      size_t count (0);
      std::list<job_id_t>::iterator iter (container_.begin());
      while (iter != container_.end())
      {
        if (item == *iter)
        {
          iter = container_.erase (iter);
          ++count;
        }
        else
        {
          ++iter;
        }
      }
      return count;
    }

    std::list<job_id_t> CoallocationScheduler::locked_job_id_list::get_and_clear()
    {
      std::lock_guard<std::mutex> const _ (mtx_);

      std::list<job_id_t> ret;
      std::swap (ret, container_);
      return ret;
    }

    void CoallocationScheduler::add_worker
      ( worker_id_t const& workerId
      , capabilities_set_t const& cpbset
      , unsigned long allocated_shared_memory_size
      , std::string const& hostname
      , fhg::com::p2p::address_t const& address
      )
    {
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);
      _worker_manager.add_worker
        ( workerId
        , cpbset
        , allocated_shared_memory_size
        , hostname
        , address
      );
    }

    void CoallocationScheduler::delete_worker_TESTING_ONLY
      (worker_id_t const& worker)
    {
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);
      _worker_manager.delete_worker (worker);
    }

    void CoallocationScheduler::acknowledge_job_sent_to_worker
      (job_id_t const& job_id, fhg::com::p2p::address_t const& source)
    {
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      auto const worker
        ( fhg::util::boost::get_or_throw<std::runtime_error>
            ( _worker_manager.worker_by_address (source)
            , "received job submission ack from unknown worker"
            )
        );

      _worker_manager.acknowledge_job_sent_to_worker (job_id, worker->second);
    }

    bool CoallocationScheduler::cancel_job_and_siblings
      ( job_id_t const& job_id
      , bool cancel_already_requested
      , std::function<void (fhg::com::p2p::address_t const&)> send_cancel
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      std::unordered_set<worker_id_t> workers_to_cancel;

      //! \note: it might happen that the job has no reservation yet
      auto const it (allocation_table_.find (job_id));
      if (it != allocation_table_.end())
      {
        workers_to_cancel = _worker_manager.find_subm_or_ack_workers
                              (job_id, it->second->workers());
      }

      if (workers_to_cancel.empty())
      {
        return false;
      }

      if (cancel_already_requested)
      {
        return true;
      }

      for (worker_id_t const& w : workers_to_cancel)
      {
        send_cancel (_worker_manager.address_by_worker (w).get()->second);
      }

      return true;
    }

    void CoallocationScheduler::notify_submitted_or_acknowledged_workers
      ( job_id_t const& job_id
      , std::function<void ( fhg::com::p2p::address_t const&)> notify_workers
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_mtx_worker_man);

      std::unordered_set<worker_id_t> workers;

      //! \note: it might happen that the job has no reservation yet
      auto const it (allocation_table_.find (job_id));
      if (it != allocation_table_.end())
      {
        workers = _worker_manager.find_subm_or_ack_workers
                    (job_id, it->second->workers());
      }

      for (worker_id_t const& w : workers)
      {
        notify_workers (_worker_manager.address_by_worker (w).get()->second);
      }
    }

    std::unordered_set<sdpa::job_id_t> CoallocationScheduler::delete_or_cancel_worker_jobs
      ( worker_id_t const& worker_id
      , std::function<Job* (sdpa::job_id_t const&)> get_job
      , std::function<void (fhg::com::p2p::address_t const&, job_id_t const&)> cancel_worker_job
      , std::lock_guard<std::mutex> const&
      , std::lock_guard<std::mutex> const&
      )
    {
      std::unordered_set<sdpa::job_id_t> jobs_to_reschedule
        (_worker_manager.pending_jobs (worker_id));

      std::unordered_set<sdpa::job_id_t> jobs_to_cancel
        (_worker_manager.submitted_or_acknowledged_jobs (worker_id));

      for (job_id_t const& jobId : jobs_to_cancel)
      {
        Job* const pJob = get_job (jobId);
        fhg_assert (pJob);

        auto const reservation (allocation_table_.at (jobId).get());
        pJob->Reschedule();

        //! \note would never be set otherwise (function is only
        //! called after a worker died)
        reservation->mark_as_canceled_if_no_result_stored_yet (worker_id);

        bool const cancel_already_requested
          ( pJob->getStatus() == sdpa::status::CANCELING
          || pJob->getStatus() == sdpa::status::CANCELED
          );

        if ( !reservation->apply_to_workers_without_result
               ( [this, &jobId, &cancel_already_requested, &cancel_worker_job]
                 (worker_id_t const& wid)
                 {
                   if (!cancel_already_requested)
                   {
                     cancel_worker_job
                       (_worker_manager.address_by_worker (wid).get()->second, jobId);
                   }
                 }
               )
           )
        {
          jobs_to_reschedule.emplace (jobId);
        }
      }

      return jobs_to_reschedule;
    }

    Workers_implementation_and_transfer_cost CoallocationScheduler::find_assignment
      ( Requirements_and_preferences const& requirements_and_preferences
      , std::lock_guard<std::mutex> const&
      ) const
    {
      size_t const num_required_workers
        (requirements_and_preferences.numWorkers());

      if (_worker_manager.number_of_workers() < num_required_workers)
      {
        return std::make_tuple<WorkerSet, Implementation, double>
          ({}, boost::none, 0.0);
      }

      bounded_priority_queue_t bpq (num_required_workers);

      for (auto const& worker_class : _worker_manager.classes_and_workers())
      {
        auto const matching_degree_and_implementation
          ( match_requirements_and_preferences
              (requirements_and_preferences, worker_class.first)
          );

        if (!matching_degree_and_implementation.first)
        {
          continue;
        }

        for (auto& worker_id : worker_class.second)
        {
          double cost_assigned_jobs;
          unsigned long shared_memory_size;
          double last_time_idle;
          double transfer_cost;

          std::tie (cost_assigned_jobs, shared_memory_size, last_time_idle, transfer_cost)
            = _worker_manager.costs_memory_size_and_last_idle_time
                (worker_id, requirements_and_preferences);

          if ( requirements_and_preferences.shared_memory_amount_required()
             > shared_memory_size
             )
            { continue; }

          double const total_cost
            ( cost_assigned_jobs
            + requirements_and_preferences.computational_cost()
            + transfer_cost
            );

          bpq.emplace ( total_cost
                      , -1.0 * matching_degree_and_implementation.first.get()
                      , shared_memory_size
                      , last_time_idle
                      , worker_id
                      , matching_degree_and_implementation.second
                      , transfer_cost
                      );
        }
      }

      if (bpq.size() == num_required_workers)
      {
        return bpq.assigned_workers_and_implementation();
      }

      return std::make_tuple<WorkerSet, Implementation, double>
        ({}, boost::none, 0.0);
    }

    std::pair<boost::optional<double>, boost::optional<std::string>>
      CoallocationScheduler::match_requirements_and_preferences
        ( Requirements_and_preferences const& requirements_and_preferences
        , std::set<std::string> const& capabilities
        ) const
    {
      for ( we::type::Requirement const& req
          : requirements_and_preferences.requirements()
          )
      {
        if (!capabilities.count (req.value()))
        {
          return std::make_pair (boost::none, boost::none);
        }
      }

      auto const preferences (requirements_and_preferences.preferences());

      if (preferences.empty())
      {
        return std::make_pair
          ( 1.0 / (capabilities.size() + 1.0)
          , boost::none
          );
      }

      auto const preference
        ( std::find_if ( preferences.cbegin()
                       , preferences.cend()
                       , [&] (Preferences::value_type const& pref)
                         {
                           return capabilities.count (pref);
                         }
                       )
        );

      if (preference == preferences.cend())
      {
        return std::make_pair (boost::none, boost::none);
      }

      boost::optional<double> matching_req_and_pref_deg
        ( ( std::distance (preference, preferences.end())
          + 1.0
          )
          /
          (capabilities.size() + preferences.size() + 1.0)
        );

      return std::make_pair (matching_req_and_pref_deg, *preference);
    }
  }
}
