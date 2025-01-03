// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <sdpa/daemon/Implementation.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/types.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <set>
#include <string>

namespace sdpa
{
  namespace daemon
  {
    namespace scheduler
    {
      class Reservation
      {
      public:
        Reservation() = delete;
        Reservation (Reservation const&) = delete;
        Reservation (Reservation&&) = delete;
        Reservation& operator= (Reservation const&) = delete;
        Reservation& operator= (Reservation&&) = delete;
        ~Reservation() = default;

        Reservation ( std::set<worker_id_t> const& workers
                    , Implementation const& implementation
                    , Preferences const& preferences
                    , double cost
                    );

        void replace_worker
          ( worker_id_t const& current_worker
          , worker_id_t const& new_worker
          , Implementation const& implementation
          , std::function<bool (std::string const& capability)> const&
              supports_implementation
          );

        std::set<worker_id_t> workers() const;
        Implementation implementation() const;
        Preferences preferences() const;

        double cost() const;

      private:
        std::set<worker_id_t> _workers;
        Implementation _implementation;
        Preferences _preferences;
        double _cost;

      public:
        //! \todo move to job statemachine instead: is duplicated
        //! state and is irrelevant to the scheduler. instances of
        //! this class should probably be deleted as soon as the
        //! workers are served, with the knowledge of where a job is
        //! served to being somewhere else (as it already has to be
        //! somewhere)

        void store_result (worker_id_t const&, terminal_state);
        void mark_as_canceled_if_no_result_stored_yet (worker_id_t const&);

        ::boost::optional<job_result_type>
          get_aggregated_results_if_all_terminated() const;

        bool apply_to_workers_without_result
          (std::function <void (worker_id_t const&)>) const;

        bool is_canceled() const;

      private:
        job_result_type _results;
      };
    }
  }
}
