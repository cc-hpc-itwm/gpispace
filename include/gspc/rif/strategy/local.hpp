// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rif/entry_point.hpp>

#include <filesystem>
#include <optional>

#include <exception>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>




      namespace gspc::rif::strategy::local
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
