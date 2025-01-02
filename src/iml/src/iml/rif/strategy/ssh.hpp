// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/rif/EntryPoint.hpp>

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
        namespace ssh
        {
          std::unordered_map<std::string, std::exception_ptr>
            bootstrap ( std::vector<std::string> const& hostnames
                      , ::boost::optional<unsigned short> const& port
                      , std::string const& register_host
                      , unsigned short register_port
                      , ::boost::filesystem::path const& binary
                      , std::vector<std::string> const& parameters
                      , std::ostream&
                      );
          std::pair < std::unordered_set<std::string>
                    , std::unordered_map<std::string, std::exception_ptr>
                    > teardown
            ( std::unordered_map<std::string, EntryPoint> const&
            , std::vector<std::string> const& parameters
            );
        }
      }
    }
  }
}
