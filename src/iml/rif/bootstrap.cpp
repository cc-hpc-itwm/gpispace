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
      , boost::optional<unsigned short> const& port
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
