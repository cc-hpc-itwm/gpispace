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

#include <string>

namespace fhg
{
  namespace util
  {
    template<typename IT>
      inline boost::optional<IT>
    generic_starts_with ( IT pos_p, const IT end_p
                        , IT pos_x, const IT end_x
                        )
    {
      while (pos_p != end_p && pos_x != end_x)
        {
          if (*pos_p != *pos_x)
            {
              return boost::none;
            }

          ++pos_p;
          ++pos_x;
        }

      if (pos_p == end_p)
        {
          return pos_x;
        }

      return boost::none;
    }

    inline std::string
      remove_prefix (std::string const& p, std::string const &x)
    {
      boost::optional<std::string::const_iterator> const pos
        (generic_starts_with (p.begin(), p.end(), x.begin(), x.end()));

      if (pos)
      {
        return std::string (*pos, x.end());
      }

      return x;
    }

    inline bool
    starts_with (const std::string & p, const std::string & x)
    {
      return !!generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
    }

    inline bool
    ends_with (const std::string & s, const std::string & x)
    {
      return !!generic_starts_with (s.rbegin(), s.rend(), x.rbegin(), x.rend());
    }
  }
}
