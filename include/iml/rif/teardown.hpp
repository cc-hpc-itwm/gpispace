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

#pragma once

#include <iml/detail/dllexport.hpp>
#include <iml/rif/EntryPoints.hpp>

#include <exception>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace iml
{
  namespace rif
  {
    //! The result of a \c teardown() call.
    struct IML_DLLEXPORT TeardownResult
    {
      //! The hostnames of all hosts that have successfully torn down
      //! their RIF daemon.
      std::unordered_set<std::string> successful_hosts;
      //! Inverse to \c successful_hosts, the exception that occurred
      //! on every failing host.
      std::unordered_map<std::string, std::exception_ptr> failures_by_host;
    };

    //! Tear down the previously started RIF daemons on \a
    //! entry_points using the strategy named \a strategy with the
    //! accompanying \a strategy_parameters.
    //! \see bootstrap()
    IML_DLLEXPORT TeardownResult teardown
      ( EntryPoints const& entry_point
      , std::string const& strategy
      , std::vector<std::string> const& strategy_parameters = {}
      );
  }
}
