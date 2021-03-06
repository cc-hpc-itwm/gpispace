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

#pragma once

#include <rif/entry_point.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <exception>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace local
      {
        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& hostnames
                    , boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , boost::filesystem::path const& binary
                    , std::vector<std::string> const& parameters
                    , std::ostream&
                    );
        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
          ( std::unordered_map<std::string, fhg::rif::entry_point> const&
          , std::vector<std::string> const& parameters
          );
      }
    }
  }
}
