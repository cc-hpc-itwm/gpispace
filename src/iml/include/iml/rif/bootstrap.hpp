// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
