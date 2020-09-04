// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <utility>

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

        separated_argument_list_parser (decltype (_sections) sections)
          : _sections (std::move (sections))
        {}
        separated_argument_list_parser ( std::string open
                                       , std::string close
                                       , std::string option
                                       )
          : _sections ({{open, {close, option}}})
        {}

        std::vector<boost::program_options::option>
          operator() (std::vector<std::string>& args) const
        {
          auto const pos (_sections.find (args[0]));

          if (pos == _sections.end())
          {
            return {};
          }

          auto begin (std::next (args.begin()));
          auto const end (std::find (begin, args.end(), pos->second.first));

          std::vector<std::string> values (begin, end);
          args.erase
            (args.begin(), end == args.end() ? end : std::next (end));

          if (values.empty())
          {
            return {};
          }

          //! \note workaround for https://svn.boost.org/trac/boost/ticket/11645
          boost::program_options::option option (pos->second.second, values);
          option.position_key = -1;
          return {option};
        }
      };
    }
  }
}
