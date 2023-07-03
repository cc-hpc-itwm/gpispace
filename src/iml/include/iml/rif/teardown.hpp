// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
