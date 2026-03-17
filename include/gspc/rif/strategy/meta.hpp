// Copyright (C) 2015-2017,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rif/entry_point.hpp>
#include <gspc/rpc/function_description.hpp>

#include <string>
#include <vector>

#include <filesystem>
#include <optional>

#include <unordered_map>
#include <unordered_set>



    namespace gspc::rif::strategy
    {
      FHG_RPC_FUNCTION_DESCRIPTION ( bootstrap_callback
                                   , void ( std::string register_key
                                          , std::string hostname
                                          , entry_point rif_entry_point
                                          )
                                   );

      std::vector<std::string> available_strategies();

      std::tuple < std::unordered_map<std::string, gspc::rif::entry_point>
                 , std::unordered_map<std::string, std::exception_ptr>
                 , std::unordered_map<std::string, std::string>
                 > bootstrap
        ( std::string const& strategy
        , std::vector<std::string> const& hostnames
        , std::optional<unsigned short> const& port
        , std::filesystem::path const& gspc_home
        , std::vector<std::string> const& parameters
        , std::ostream&
        );
      std::pair < std::unordered_set<std::string>
                , std::unordered_map<std::string, std::exception_ptr>
                > teardown
        ( std::string const& strategy
        , std::unordered_map<std::string, gspc::rif::entry_point> const& entry_points
        , std::vector<std::string> const& parameters
        );
    }
