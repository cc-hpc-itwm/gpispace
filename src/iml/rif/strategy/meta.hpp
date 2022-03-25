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

#include <iml/rif/EntryPoint.hpp>
#include <util-rpc/function_description.hpp>

#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <unordered_map>
#include <unordered_set>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      using ::iml::rif::EntryPoint;
      namespace strategy
      {
        FHG_RPC_FUNCTION_DESCRIPTION ( bootstrap_callback
                                     , void ( std::string register_key
                                            , std::string hostname
                                            , EntryPoint rif_entry_point
                                            )
                                     );

        std::vector<std::string> available_strategies();

        std::tuple < std::unordered_map<std::string, EntryPoint>
                   , std::unordered_map<std::string, std::exception_ptr>
                   , std::unordered_map<std::string, std::string>
                   > bootstrap
          ( std::string const& strategy
          , std::vector<std::string> const& hostnames
          , ::boost::optional<unsigned short> const& port
          , std::vector<std::string> const& parameters
          , std::ostream&
          );
        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
          ( std::string const& strategy
          , std::unordered_map<std::string, EntryPoint> const& entry_points
          , std::vector<std::string> const& parameters
          );
      }
    }
  }
}
