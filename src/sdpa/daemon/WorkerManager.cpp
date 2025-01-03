// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/daemon/WorkerManager.hpp>

#include <sdpa/types.hpp>

#include <fhg/assert.hpp>

#include <fhgcom/address.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/make_optional.hpp>

#include <boost/range/algorithm.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace sdpa
{
  namespace daemon
  {
    std::unordered_set<worker_id_t> WorkerManager::find_subm_or_ack_workers
      (job_id_t const& job_id, std::set<worker_id_t> const& assigned_workers) const
    {
      std::unordered_set<worker_id_t> submitted_or_ack_workers;

      for (auto const& wid : assigned_workers)
      {
        auto const worker (worker_map_.at (wid));

        if ( worker.submitted_.count (job_id)
           || worker.acknowledged_.count (job_id)
           )
        {
          submitted_or_ack_workers.emplace (wid);
        }
      }

      return submitted_or_ack_workers;
    }

    void WorkerManager::add_worker ( worker_id_t const& workerId
                                   , Capabilities const& cpbSet
                                   , unsigned long allocated_shared_memory_size
                                   , std::string const& hostname
                                   , fhg::com::p2p::address_t const& address
                                   )
    {
      if (worker_map_.count (workerId) != 0)
      {
        throw std::runtime_error ("worker '" + workerId + "' already exists");
      }
      worker_connections_.left.insert ({workerId, address});
      auto result = worker_map_.emplace ( workerId
                                        , Worker ( cpbSet
                                                 , allocated_shared_memory_size
                                                 , hostname
                                                 )
                                        );

      auto const equiv_class (result.first->second.capability_names_);
      worker_equiv_classes_[equiv_class].add_worker_entry (result.first);

      // allow to steal from itself
      worker_equiv_classes_.at (equiv_class)
        ._stealing_allowed_classes.emplace (result.first->second.capability_names_);

      _classes_and_worker_ids[equiv_class].emplace (workerId);
    }

    void WorkerManager::delete_worker (worker_id_t const& workerId)
    {
      auto const worker (worker_map_.find (workerId));
      fhg_assert (worker != worker_map_.end(), "Worker not found when deletion was requested!");

      auto const equiv_class_name (worker->second.capability_names_);
      auto const equivalence_class
        (worker_equiv_classes_.find (equiv_class_name));

      equivalence_class->second.remove_worker_entry (worker);

      _classes_and_worker_ids.at (equiv_class_name).erase (workerId);

      if (_classes_and_worker_ids.at (equiv_class_name).empty())
      {
        worker_equiv_classes_.erase (equivalence_class);
        _classes_and_worker_ids.erase (equiv_class_name);

        for (auto& worker_class : worker_equiv_classes_)
        {
          worker_class.second._stealing_allowed_classes.erase
            (equiv_class_name);
        }
      }

      worker_map_.erase (worker);
      worker_connections_.left.erase (workerId);
    }

    void WorkerManager::assign_job_to_worker
      ( job_id_t const& job_id
      , worker_id_t const& worker_id
      , double cost
      , Preferences const& preferences
      )
    {
      auto worker (worker_map_.find (worker_id));
      fhg_assert (worker != worker_map_.end());
      assign_job_to_worker (job_id, worker, cost, preferences);
    }

    void WorkerManager::assign_job_to_worker
      ( job_id_t const& job_id
      , worker_iterator worker
      , double cost
      , Preferences const& preferences
      )
    {
      worker->second.assign (job_id, cost);

      auto& worker_class
        (worker_equiv_classes_.at (worker->second.capability_names_));
      worker_class.inc_pending_jobs (1);

      worker_class._idle_workers.erase (worker->first);

      worker_class.allow_classes_matching_preferences_stealing
        (worker_equiv_classes_, preferences);
    }

    void WorkerManager::submit_job_to_worker (job_id_t const& job_id, worker_id_t const& worker_id)
    {
      auto worker (worker_map_.find (worker_id));
      worker->second.submit (job_id);
      auto& equivalence_class
        (worker_equiv_classes_.at (worker->second.capability_names_));

      equivalence_class.dec_pending_jobs (1);
      equivalence_class.inc_running_jobs (1);

      equivalence_class._idle_workers.erase (worker->first);
      equivalence_class._num_free_workers--;
    }

    void WorkerManager::acknowledge_job_sent_to_worker ( job_id_t const& job_id
                                                       , worker_id_t const& worker_id
                                                       )
    {
      worker_map_.at (worker_id).acknowledge (job_id);
    }

    void WorkerManager::delete_job_from_worker ( job_id_t const& job_id
                                               , worker_id_t const& worker_id
                                               , double cost
                                               )
    {
      auto worker (worker_map_.find (worker_id));
      delete_job_from_worker (job_id, worker, cost);
    }

    void WorkerManager::delete_job_from_worker
      ( job_id_t const& job_id
      , worker_iterator worker
      , double cost
      )
    {
      if (worker != worker_map_.end())
      {
        auto& equivalence_class
          (worker_equiv_classes_.at (worker->second.capability_names_));

        if (worker->second.pending_.count (job_id))
        {
          worker->second.delete_pending_job (job_id, cost);
          equivalence_class.dec_pending_jobs (1);
        }
        else
        {
          worker->second.delete_submitted_job (job_id, cost);
          equivalence_class.dec_running_jobs (1);
          equivalence_class._num_free_workers++;
        }

        if ( !worker->second.has_running_jobs()
           && !worker->second.has_pending_jobs()
           )
        {
          equivalence_class._idle_workers.emplace (worker->first);
        }
      }
    }

    ::boost::optional<WorkerManager::worker_connections_t::right_iterator>
      WorkerManager::worker_by_address (fhg::com::p2p::address_t const& address)
    {
      WorkerManager::worker_connections_t::right_iterator it
        (worker_connections_.right.find (address));
      return ::boost::make_optional (it != worker_connections_.right.end(), it);
    }

    ::boost::optional<WorkerManager::worker_connections_t::left_iterator>
      WorkerManager::address_by_worker (std::string const& worker)
    {
      WorkerManager::worker_connections_t::left_iterator it
        (worker_connections_.left.find (worker));
      return ::boost::make_optional (it != worker_connections_.left.end(), it);
    }

    void WorkerManager::WorkerEquivalenceClass::inc_pending_jobs (unsigned int k)
    {
      _num_pending_jobs += k;
    }

    void WorkerManager::WorkerEquivalenceClass::dec_pending_jobs (unsigned int k)
    {
      fhg_assert ( _num_pending_jobs >= k
                 , "The number of pending jobs of a group of workers cannot be a negative number"
                 );

      _num_pending_jobs -= k;
    }

    void WorkerManager::WorkerEquivalenceClass::inc_running_jobs (unsigned int k)
    {
      _num_running_jobs += k;
    }

    void WorkerManager::WorkerEquivalenceClass::dec_running_jobs (unsigned int k)
    {
      fhg_assert ( _num_running_jobs >= k
                 , "The number of running jobs of a group of workers cannot be a negative number"
                 );

      _num_running_jobs -= k;
    }

    unsigned int WorkerManager::WorkerEquivalenceClass::num_pending_jobs() const
    {
      return _num_pending_jobs;
    }

    unsigned int WorkerManager::WorkerEquivalenceClass::num_running_jobs() const
    {
      return _num_running_jobs;
    }

    void WorkerManager::WorkerEquivalenceClass::add_worker_entry
      (worker_iterator worker)
    {
      inc_pending_jobs (worker->second.pending_.size());
      inc_running_jobs ( worker->second.submitted_.size()
                       + worker->second.acknowledged_.size()
                       );
      _idle_workers.emplace (worker->first);
      _num_free_workers++;
    }

    void WorkerManager::WorkerEquivalenceClass::remove_worker_entry
      (worker_iterator worker)
    {
      dec_pending_jobs (worker->second.pending_.size());
      auto const num_running_jobs
        ( worker->second.submitted_.size()
        + worker->second.acknowledged_.size()
        );

      if (num_running_jobs != 0)
      {
        dec_running_jobs (num_running_jobs);
      }
      else
      {
        _num_free_workers--;
      }

      _idle_workers.erase (worker->first);
    }

    void WorkerManager::WorkerEquivalenceClass::allow_classes_matching_preferences_stealing
      ( std::map<std::set<std::string>, WorkerEquivalenceClass> const& worker_classes
      , Preferences const& preferences
      )
    {
      if (preferences.empty())
      {
        return;
      }

      for (auto const& worker_class : worker_classes)
      {
        if (std::any_of ( preferences.begin()
                        , preferences.end()
                        , [&worker_class] (std::string const& preference)
                          { return worker_class.first.count (preference); }
                        )
           )
        {
          _stealing_allowed_classes.emplace (worker_class.first);
        }
      }
    }

    void WorkerManager::steal_work
      ( CostModel cost_model
      , std::function<scheduler::Reservation* (job_id_t const&)> reservation
      )
    {
      for (auto const& weqc : worker_equiv_classes_)
      {
        if (weqc.second.num_pending_jobs() == 0)
        {
          continue;
        }

        std::unordered_set<worker_id_t> thieves;

        auto const& worker_classes (worker_equiv_classes_);
        for (auto const& cls : weqc.second._stealing_allowed_classes)
        {
          thieves.insert ( worker_classes.at (cls)._idle_workers.begin()
                         , worker_classes.at (cls)._idle_workers.end()
                         );
        }

        if (thieves.empty())
        {
          continue;
        }

        auto const cost
          ( [&] (job_id_t const& job_id)
            {
              return fhg::util::visit
                ( cost_model
                , [&] (UsingCosts)
                  {
                    return reservation (job_id)->cost();
                  }
                , [&] (NotUsingCosts)
                  {
                    return 0.0;
                  }
                );
            }
          );

        auto const comp
          ( [&] (worker_iterator const& lhs, worker_iterator const& rhs)
            {
              return fhg::util::visit
                ( cost_model
                , [&] (UsingCosts)
                  {
                    return lhs->second.cost_assigned_jobs()
                      < rhs->second.cost_assigned_jobs();
                  }
                , [&] (NotUsingCosts)
                  {
                    return lhs->second.pending_.size()
                      < rhs->second.pending_.size();
                  }
                );
            }
          );

        std::priority_queue < worker_iterator
                            , std::vector<worker_iterator>
                            , decltype (comp)
                            > to_steal_from (comp);

        for (worker_id_t const& w : _classes_and_worker_ids.at (weqc.first))
        {
          auto const& it (worker_map_.find (w));
          fhg_assert (it != worker_map_.end());
          Worker const& worker (it->second);

          if (worker.stealing_allowed())
          {
            to_steal_from.emplace (it);
          }
        }

        while (!(thieves.empty() || to_steal_from.empty()))
        {
          auto const richest (to_steal_from.top());
          worker_iterator const& thief (worker_map_.find (*thieves.begin()));
          Worker& richest_worker (richest->second);

          auto it_job
            ( fhg::util::visit
                ( cost_model
                , [&] (UsingCosts)
                  {
                    return std::max_element
                      ( richest_worker.pending_.begin()
                      , richest_worker.pending_.end()
                      , [&reservation] ( job_id_t const& r
                                       , job_id_t const& l
                                       )
                        {
                          return reservation (r)->cost()
                            < reservation (l)->cost();
                        }
                      );
                  }
                , [&] (NotUsingCosts)
                  {
                    return richest_worker.pending_.begin();
                  }
                )
              );

          fhg_assert (it_job != richest_worker.pending_.end());

          Preferences const preferences (reservation (*it_job)->preferences());

          auto const preference
            (std::find_if ( preferences.begin()
                          , preferences.end()
                          , [&] (std::string const& pref)
                            {
                              return thief->second.hasCapability (pref);
                            }
                          )
            );

          if (preferences.empty() || preference != preferences.end())
          {
            reservation (*it_job)->replace_worker
              ( richest->first
              , thief->first
              , FHG_UTIL_MAKE_OPTIONAL (!preferences.empty(), *preference)
              , [&thief] (std::string const& cpb)
                {
                  return thief->second.hasCapability (cpb);
                }
              );

            auto const job_cost (cost (*it_job));
            assign_job_to_worker (*it_job, thief, job_cost, preferences);
            delete_job_from_worker (*it_job, richest, job_cost);

            to_steal_from.pop();

            if (richest_worker.stealing_allowed())
            {
              to_steal_from.emplace (richest);
            }
          }

          thieves.erase (thieves.begin());
        }
      }
    }

    unsigned long WorkerManager::num_free_workers
      (std::set<std::string> const& cls) const
    {
      return worker_equiv_classes_.at (cls)._num_free_workers;
    }

    bool WorkerManager::all_free (std::set<worker_id_t> const& workers) const
    {
      return std::all_of
        ( workers.begin()
        , workers.end()
        , [this] (worker_id_t const& worker)
          {
            return worker_map_.count (worker)
              && !worker_map_.at (worker).isReserved();
          }
        );
    }

    std::unordered_set<job_id_t> WorkerManager::pending_jobs
      (worker_id_t const& worker_id) const
    {
      auto const worker (worker_map_.at (worker_id));
      return std::unordered_set<job_id_t>
        (worker.pending_.begin(), worker.pending_.end());
    }

    std::unordered_set<job_id_t> WorkerManager::submitted_or_acknowledged_jobs
      (worker_id_t const& worker_id) const
    {
      auto const worker (worker_map_.at (worker_id));

      std::unordered_set<sdpa::job_id_t> sub_or_ack_jobs;
      std::set_union ( worker.submitted_.begin()
                     , worker.submitted_.end()
                     , worker.acknowledged_.begin()
                     , worker.acknowledged_.end()
                     , std::inserter (sub_or_ack_jobs, sub_or_ack_jobs.begin())
                     );

      return sub_or_ack_jobs;
    }

    std::tuple<double, unsigned long, double, double>
      WorkerManager::costs_memory_size_and_last_idle_time
        ( worker_id_t const& worker_id
        , Requirements_and_preferences const& requirements_and_preferences
        ) const
    {
      auto const& worker (worker_map_.at (worker_id));

      return std::make_tuple
        ( worker.cost_assigned_jobs()
        , worker._allocated_shared_memory_size
        , worker._last_time_idle
        , requirements_and_preferences.transfer_cost() (worker._hostname)
        );
    }

    unsigned long WorkerManager::number_of_workers() const
    {
      return worker_map_.size();
    }

    std::map<std::set<std::string>, std::unordered_set<worker_id_t>> const&
      WorkerManager::classes_and_workers() const
    {
      return _classes_and_worker_ids;
    }

    boost::optional<job_id_t> WorkerManager::get_next_worker_pending_job_to_submit
      (worker_id_t const& worker_id)
    {
      auto const next_pending_job_to_submit
        (worker_map_.at (worker_id).get_next_pending_job_to_submit());

      if (next_pending_job_to_submit)
      {
        submit_job_to_worker (*next_pending_job_to_submit, worker_id);
      }

      return next_pending_job_to_submit;
    }

    unsigned long WorkerManager::num_pending_jobs (std::set<std::string> const& cls)
    {
      return worker_equiv_classes_.at (cls).num_pending_jobs();
    }

    unsigned long WorkerManager::num_running_jobs (std::set<std::string> const& cls)
    {
      return worker_equiv_classes_.at (cls).num_running_jobs();
    }
  }
}
