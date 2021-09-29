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

#include <boost/program_options/option.hpp>

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace program_options
    {
      struct separated_argument_list_parser
      {
        std::map< std::string              // open
                , std::pair< std::string   // close
                           , std::string   // option
                           >
                > _sections;

        separated_argument_list_parser (decltype (_sections));
        separated_argument_list_parser ( std::string open
                                       , std::string close
                                       , std::string option
                                       );

        std::vector<boost::program_options::option>
          operator() (std::vector<std::string>& args) const;
      };
    }
  }
}
