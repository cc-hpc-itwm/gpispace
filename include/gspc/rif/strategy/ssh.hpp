// Copyright (C) 2015,2017,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rif/entry_point.hpp>

#include <string>
#include <vector>

#include <filesystem>
#include <optional>

#include <unordered_map>
#include <unordered_set>




      namespace gspc::rif::strategy::ssh
      {
        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& hostnames
                    , std::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , std::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    );
        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
          ( std::unordered_map<std::string, gspc::rif::entry_point> const&
          , std::vector<std::string> const& parameters
          );
      }
