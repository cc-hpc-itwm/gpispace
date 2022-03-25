// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <sdpa/daemon/scheduler/CostAwareWithWorkStealingStrategy.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/requirements_and_preferences.hpp>
#include <we/type/Requirement.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <iterator>
#include <list>
#include <set>
#include <string>
#include <thread>
#include <tuple>

namespace
{
  template<typename Exception, typename T, typename... Args>
    T get_or_throw (::boost::optional<T> const& optional, Args&&... args)
  {
    if (!optional)
    {
      throw Exception (args...);
    }

    return optional.get();
  }
}

namespace sdpa
{
  namespace daemon
  {
    CostAwareWithWorkStealingStrategy::CostAwareWithWorkStealingStrategy
        ( std::function<Requirements_and_preferences (sdpa::job_id_t const&)>
            requirements_and_preferences
        , WorkerManager& worker_manager
        )
      : _requirements_and_preferences (requirements_and_preferences)
      , _worker_manager (worker_manager)
    {}

    void CostAwareWithWorkStealingStrategy::locked_job_id_list::push
      (job_id_t const& item)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      container_.emplace_back (item);
    }

    void CostAwareWithWorkStealingStrategy::locked_job_id_list::push
      (std::list<job_id_t> const& items)
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      container_.insert (container_.end(), items.begin(), items.end());
    }

    size_t CostAwareWithWorkStealingStrategy::locked_job_id_list::erase
      (job_id_t const& item)
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

    std::list<job_id_t>
      CostAwareWithWorkStealingStrategy::locked_job_id_list::get_and_clear()
    {
      std::lock_guard<std::mutex> const _ (mtx_);
      std::list<job_id_t> ret;
      std::swap (ret, container_);
      return ret;
    }

    std::pair<::boost::optional<double>,::boost::optional<std::string>>
      CostAwareWithWorkStealingStrategy::match_requirements_and_preferences
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
          return std::make_pair (::boost::none, ::boost::none);
        }
      }

      auto const preferences (requirements_and_preferences.preferences());

      if (preferences.empty())
      {
        return std::make_pair
          ( 1.0 / (capabilities.size() + 1.0)
          , ::boost::none
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
        return std::make_pair (::boost::none, ::boost::none);
      }

      ::boost::optional<double> matching_req_and_pref_deg
        ( ( std::distance (preference, preferences.end())
          + 1.0
          )
          /
          (capabilities.size() + preferences.size() + 1.0)
        );

      return std::make_pair (matching_req_and_pref_deg, *preference);
    }

    void CostAwareWithWorkStealingStrategy::delete_job
      (sdpa::job_id_t const& job)
    {
      _pending_jobs.remove (job);
      _jobs_to_schedule.erase (job);
    }

    void CostAwareWithWorkStealingStrategy::submit_job
      (sdpa::job_id_t const& jobId)
    {
      _jobs_to_schedule.push (jobId);
    }

    void CostAwareWithWorkStealingStrategy::acknowledge_job_sent_to_worker
      (job_id_t const& job_id, fhg::com::p2p::address_t const& source)
    {
      std::lock_guard<std::mutex> const lock_worker_man
        (_worker_manager._mutex);

      auto const worker
        ( get_or_throw<std::runtime_error>
            ( _worker_manager.worker_by_address (source)
            , "received job submission ack from unknown worker"
            )
        );

      _worker_manager.acknowledge_job_sent_to_worker (job_id, worker->second);
    }

    void CostAwareWithWorkStealingStrategy::steal_work
      (CostModel const& cost_model)
    {
      std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_worker_manager._mutex);

      _worker_manager.steal_work
        ( cost_model
        , [this] (job_id_t const& job)
          {
            return allocation_table_.at (job).get();
          }
        );
    }

    ::boost::optional<job_result_type> CostAwareWithWorkStealingStrategy::
      store_individual_result_and_get_final_if_group_finished
        ( fhg::com::p2p::address_t const& worker_addr
        , job_id_t const& job_id
        , terminal_state const& result
        )
    {
      std::lock_guard<std::mutex> const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error ("store_result: unknown job");
      }

      std::lock_guard<std::mutex> const lock_worker_man
        (_worker_manager._mutex);

      auto const worker
        ( get_or_throw<std::invalid_argument>
            ( _worker_manager.worker_by_address (worker_addr)
            , "attempting to store job result from unknown worker!"
            )
        );

      it->second->store_result (worker->second, result);

      return it->second->get_aggregated_results_if_all_terminated();
    }

    void CostAwareWithWorkStealingStrategy::notify_submitted_or_acknowledged_workers
      ( job_id_t const& job_id
      , std::function<void ( fhg::com::p2p::address_t const&)> notify_workers
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_worker_manager._mutex);

      std::unordered_set<worker_id_t> workers;

      //! \note: it might happen that the job has no reservation yet
      auto const it (allocation_table_.find (job_id));
      if (it != allocation_table_.end())
      {
        workers = _worker_manager.find_subm_or_ack_workers (job_id, it->second->workers());
      }

      for (worker_id_t const& w : workers)
      {
        notify_workers (_worker_manager.address_by_worker (w).get()->second);
      }
    }

    void CostAwareWithWorkStealingStrategy::release_reservation
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

    void CostAwareWithWorkStealingStrategy::reschedule_worker_jobs_and_maybe_remove_worker
      ( fhg::com::p2p::address_t const& source
      , std::function<Job* (sdpa::job_id_t const&)> get_job
      , std::function<void (Job* job)> notify_job_failed
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man
        (_worker_manager._mutex);

      auto const as_worker
        (_worker_manager.worker_by_address (source));

      if (as_worker)
      {
        auto const worker_id (as_worker.get()->second);

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

          jobs_to_reschedule.emplace (jobId);
        }

        for ( job_id_t const& job_id : jobs_to_reschedule)
        {
          release_reservation (job_id, lock_alloc_table, lock_worker_man);
          delete_job (job_id);

          auto const job (get_job (job_id));

          if (job->check_and_inc_retry_counter())
          {
            submit_job (job_id);
          }
          else
          {
            notify_job_failed (job);
          }
        }

        _worker_manager.delete_worker (worker_id);
      }
    }

    void CostAwareWithWorkStealingStrategy::reschedule_worker_jobs_and_maybe_remove_worker
      ( fhg::com::p2p::address_t const& source
      , std::function<Job* (sdpa::job_id_t const&)> get_job
      , std::function<void (fhg::com::p2p::address_t const& addr, job_id_t const&)> cancel_worker_job
      , std::function<void (Job* job)> job_failed
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_worker_manager._mutex);

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

          auto const job (get_job (job_id));
          if (job->check_and_inc_retry_counter())
          {
            submit_job (job_id);
          }
          else
          {
            job_failed (job);
          }
        }

        _worker_manager.delete_worker (worker);
      }
    }

    std::unordered_set<sdpa::job_id_t> CostAwareWithWorkStealingStrategy::delete_or_cancel_worker_jobs
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

    bool CostAwareWithWorkStealingStrategy::cancel_job
      ( job_id_t const& job_id
      , std::function<void (fhg::com::p2p::address_t const&)> send_cancel
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_worker_manager._mutex);

      //! \note: it might happen that the job has no reservation yet
      auto const it (allocation_table_.find (job_id));
      if (it == allocation_table_.end())
      {
        delete_job (job_id);
        return false;
      }

      auto const& job_reservation (it->second);
      if (job_reservation->workers().size() != 1)
      {
        throw std::logic_error
          ( "A task cannot have more than one worker assigned when using a "
            "single allocation scheduler! Please use a coallocation scheduler "
            "if this was intended."
          );
      }

      auto const assigned_worker (*job_reservation->workers().cbegin());
      if ( _worker_manager.submitted_or_acknowledged_jobs
             (assigned_worker).count (job_id)
         )
      {
        send_cancel
          (_worker_manager.address_by_worker (assigned_worker).get()->second);
        return true;
      }
      else
      {
        release_reservation (job_id, lock_alloc_table, lock_worker_man);
        delete_job (job_id);
        return false;
      }
    }

    bool CostAwareWithWorkStealingStrategy::cancel_job
      ( job_id_t const& job_id
      , bool cancel_already_requested
      , std::function<void (fhg::com::p2p::address_t const&)> send_cancel
      )
    {
      std::lock_guard<std::mutex> const lock_alloc_table (mtx_alloc_table_);
      std::lock_guard<std::mutex> const lock_worker_man (_worker_manager._mutex);

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
        release_reservation (job_id, lock_alloc_table, lock_worker_man);
        delete_job (job_id);
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

    bool CostAwareWithWorkStealingStrategy::assign_job
      ( job_id_t const& jobId
      , WorkerSet const& workers
      , double cost
      , Implementation const& implementation
      , Preferences const& preferences
      , std::list<job_id_t>& jobs_to_schedule
      , std::lock_guard<std::mutex> const&
      , std::lock_guard<std::mutex> const&
      )
    {
      if ( allocation_table_.find (jobId) != allocation_table_.end())
      {
        throw std::runtime_error ("already have reservation for the job " + jobId);
      }

      try
      {
        for (auto const& worker : workers)
        {
          _worker_manager.assign_job_to_worker
            ( jobId
            , worker
            , cost
            , preferences
            );
        }

        allocation_table_.emplace
          ( jobId
          , std::make_unique<scheduler::Reservation>
              ( workers
              , implementation
              , preferences
              , cost
              )
          );

        return true;
      }
      catch (std::out_of_range const&)
      {
        for (auto const&  worker : workers)
        {
          _worker_manager.delete_job_from_worker (jobId, worker, cost);
        }

        jobs_to_schedule.push_front (jobId);

        return false;
      }
    }

    void CostAwareWithWorkStealingStrategy::PendingJobs::add
      (std::set<std::string> worker_class, job_id_t job)
    {
      _jobs[worker_class].emplace (job);
      _class.emplace (job, worker_class);
    }

    void CostAwareWithWorkStealingStrategy::PendingJobs::remove (job_id_t job)
    {
      auto worker_class (_class.find (job));
      if (worker_class != _class.end())
      {
        _jobs.at (worker_class->second).erase (job);
      }
    }

    std::map <std::set<std::string>, std::unordered_set<job_id_t>>&
      CostAwareWithWorkStealingStrategy::PendingJobs::operator()()
      {
        return _jobs;
      }
  }
}
