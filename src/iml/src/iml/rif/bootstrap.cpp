// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/rif/bootstrap.hpp>

#include <iml/rif/strategy/meta.hpp>

#include <utility>

namespace iml
{
  namespace rif
  {
    BootstrapResult bootstrap
      ( std::vector<std::string> const& hostnames
      , std::string const& strategy
      , std::vector<std::string> const& strategy_parameters
      , ::boost::optional<unsigned short> const& port
      , std::ostream& output
      )
    {
      auto result
        ( fhg::iml::rif::strategy::bootstrap
            ( strategy
            , hostnames
            , port
            , strategy_parameters
            , output
            )
        );
      return { std::move (std::get<0> (result))
             , std::move (std::get<1> (result))
             , std::move (std::get<2> (result))
             };
    }
  }
}
