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

#include <boost/optional.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace iml
{
  namespace rif
  {
    //! The result of a \c bootstrap() call.
    struct IML_DLLEXPORT BootstrapResult
    {
      //! The new entry point for every successfully bootstrapped host.
      EntryPoints entry_points;
      //! Inverse to \c entry_points, the exception that occurred on
      //! every failing host.
      std::unordered_map<std::string, std::exception_ptr> failures_by_host;
      //! The hostname as per \c hostname() executed on the host
      //! itself for every hostname given.
      std::unordered_map<std::string, std::string> detected_hostnames_by_host;
    };

    //! Bootstrap RIF daemons on every host given in \a hostnames,
    //! using the strategy named \a strategy with the accompanying \a
    //! strategy_parameters. When given, the daemons will use \a port
    //! to listen on or an automatically determined one otherwise. The
    //! strategy may print messages to \a output.
    //! \note Ignores duplicate hostnames without warning.
    //! \see teardown()
    IML_DLLEXPORT BootstrapResult bootstrap
      ( std::vector<std::string> const& hostnames
      , std::string const& strategy
      , std::vector<std::string> const& strategy_parameters = {}
      , ::boost::optional<unsigned short> const& port = ::boost::none
      , std::ostream& output = std::cout
      );
  }
}
