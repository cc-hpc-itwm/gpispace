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

#include <iterator>
#include <list>
#include <string>
#include <utility>

namespace fhg
{
  namespace util
  {
    template<typename T, typename U, typename C = std::list<U>>
      inline C split ( T const& x
                     , typename std::iterator_traits<typename T::const_iterator>::value_type const& s
                     )
    {
      C path;
      T key;

      for (typename T::const_iterator pos (x.begin()); pos != x.end(); ++pos)
      {
        if (*pos == s)
        {
          path.push_back (key);
          key.clear();
        }
        else
        {
          key.push_back (*pos);
        }
      }

      if (!key.empty())
      {
        path.push_back (key);
      }

      return path;
    }


    inline std::pair<std::string, std::string> split_string
      (std::string const& val, char sep)
    {
      std::string::size_type const pos (val.find (sep));

      return std::make_pair
        ( val.substr (0, pos)
        , pos != std::string::npos ? val.substr (pos + 1) : ""
        );
    }
  }
}
