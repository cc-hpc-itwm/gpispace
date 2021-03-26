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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_REQUIRE_ALL_IF_ONE_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_REQUIRE_ALL_IF_ONE_HPP

#include <boost/program_options.hpp>

#include <algorithm>
#include <functional>
#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      namespace program_options
      {
        using namespace ::boost::program_options;

        static
        void require_all_if_one
          (variables_map const& vm, std::initializer_list<const char*> options)
        {
          std::function<bool (const char*)> const has_option
            ( [&vm] (const char* option)
              {
                return !!vm.count (option);
              }
            );

          if ( std::any_of (options.begin(), options.end(), has_option)
             && !std::all_of (options.begin(), options.end(), has_option)
             )
          {
            std::stringstream message;
            message << "setting any option of";
            for (const char* option : options)
            {
              message << " '" << option << "'";
            }
            message << " requires setting all of them";
            throw std::invalid_argument (message.str());
          }
        }
      }
    }
  }
}

#endif
