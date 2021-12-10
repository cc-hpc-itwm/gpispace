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

#ifndef FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_HPP
#define FHG_UTIL_BOOST_PROGRAM_OPTIONS_VALIDATORS_HPP

#include <boost/program_options.hpp>

#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        template<typename T>
          inline void validate ( ::boost::any& v
                               , const std::vector<std::string>& values
                               )
        {
          ::boost::program_options::validators::check_first_occurrence (v);

          v = ::boost::any (T (::boost::program_options::validators::get_single_string (values)));
        }
      }
    }
  }
}

#endif
