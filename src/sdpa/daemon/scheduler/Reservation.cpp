// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>

#include <algorithm>
#include <fmt/core.h>
#include <stdexcept>
#include <utility>

namespace sdpa
{
  namespace daemon
  {
    namespace scheduler
    {
      Reservation::Reservation ( std::set<worker_id_t> const& workers
                               , Implementation const& implementation
                               , Preferences const& preferences
                               , double cost
                               )
        : _workers (workers)
        , _implementation (implementation)
        , _preferences (preferences)
        , _cost (cost)
      {}

      void Reservation::replace_worker
        ( worker_id_t const& current_worker
        , worker_id_t const& new_worker
        , Implementation const& implementation
        , std::function<bool (std::string const&)> const&
            supports_implementation
        )
      {
        if (!_workers.count (current_worker))
        {
          throw std::runtime_error
            ("Asked to replace the non-existent worker " + current_worker);
        }

        if (!_preferences.empty())
        {
          if (!implementation)
          {
            throw std::logic_error
              ("The implementation cannot be ::boost::none "
               "if the set of preferences is not empty!"
              );
          }
          else if ( std::find ( _preferences.begin()
                              , _preferences.end()
                              , *implementation
                              )
                  == _preferences.end()
                  )
           {
             throw std::logic_error
               ("The implementation must be set to one of "
                "the preferences stored in the reservation!"
               );
           }
        }

        if (implementation && !supports_implementation (*implementation))
        {
          throw std::runtime_error
            { fmt::format
                ( "Cannot replace worker {} with worker {}: "
                  "{} does not support the implementation {}."
                , current_worker
                , new_worker
                , new_worker
                , *implementation
                )
            };
        }

        _implementation = implementation;
        _workers.emplace (new_worker);
        _workers.erase (current_worker);
      }

      std::set<worker_id_t> Reservation::workers() const
      {
        return _workers;
      }

      Implementation Reservation::implementation() const
      {
        return _implementation;
      }

      Preferences Reservation::preferences() const
      {
        return _preferences;
      }

      double Reservation::cost() const
      {
        return _cost;
      }

      void Reservation::store_result
        (worker_id_t const& worker, terminal_state result)
      {
        if (!_results.individual_results.emplace (worker, result).second)
        {
          throw std::logic_error ("store_result: second result");
        }
        if (std::holds_alternative<JobFSM_::s_finished> (result))
        {
          _results.last_success =
            std::move (std::get<JobFSM_::s_finished> (result));
        }
      }
      void Reservation::mark_as_canceled_if_no_result_stored_yet
        (worker_id_t const& worker)
      {
        _results.individual_results.emplace (worker, JobFSM_::s_canceled());
      }

      ::boost::optional<job_result_type>
        Reservation::get_aggregated_results_if_all_terminated() const
      {
        return ::boost::make_optional
          (_results.individual_results.size() == _workers.size(), _results);
      }

      bool Reservation::apply_to_workers_without_result
        (std::function <void (worker_id_t const&)> fun) const
      {
        bool applied {false};

        for (auto const& worker : _workers)
        {
          if (!_results.individual_results.count (worker))
          {
            fun (worker);

            applied = true;
          }
        }

        return applied;
      }

      bool Reservation::is_canceled() const
      {
        return std::any_of
          ( _results.individual_results.begin()
          , _results.individual_results.end()
          , [] (std::pair<sdpa::worker_id_t, terminal_state> const& result)
            {
              return std::holds_alternative<JobFSM_::s_canceled> (result.second);
            }
          );
      }
    }
  }
}
