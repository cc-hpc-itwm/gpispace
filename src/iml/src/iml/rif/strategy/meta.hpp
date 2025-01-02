// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
