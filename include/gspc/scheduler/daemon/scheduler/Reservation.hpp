// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/scheduler/daemon/Implementation.hpp>
#include <gspc/scheduler/daemon/Job.hpp>
#include <gspc/scheduler/types.hpp>

#include <optional>

#include <functional>
#include <set>
#include <string>



    namespace gspc::scheduler::daemon::scheduler
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
                    , gspc::we::type::Preferences const& preferences
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
        gspc::we::type::Preferences preferences() const;

        double cost() const;

      private:
        std::set<worker_id_t> _workers;
        Implementation _implementation;
        gspc::we::type::Preferences _preferences;
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

        std::optional<job_result_type>
          get_aggregated_results_if_all_terminated() const;

        bool apply_to_workers_without_result
          (std::function <void (worker_id_t const&)>) const;

        bool is_canceled() const;

      private:
        job_result_type _results;
      };
    }
