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
