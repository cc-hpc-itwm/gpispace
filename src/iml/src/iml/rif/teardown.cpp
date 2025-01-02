// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/rif/teardown.hpp>

#include <iml/rif/strategy/meta.hpp>

#include <utility>

namespace iml
{
  namespace rif
  {
    TeardownResult teardown
      ( EntryPoints const& entry_points
      , std::string const& strategy
      , std::vector<std::string> const& strategy_parameters
      )
    {
      auto result
        ( fhg::iml::rif::strategy::teardown
            ( strategy
            , entry_points
            , strategy_parameters
            )
        );
      return { std::move (std::get<0> (result))
             , std::move (std::get<1> (result))
             };
    }
  }
}
