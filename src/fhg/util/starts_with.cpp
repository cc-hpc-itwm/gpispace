// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <fhg/util/starts_with.hpp>

namespace fhg
{
  namespace util
  {
    namespace
    {
      template<typename IT>
        bool generic_starts_with ( IT pos_p, IT end_p
                                 , IT pos_x, IT end_x
                                 )
      {
        while (pos_p != end_p && pos_x != end_x && *pos_p == *pos_x)
        {
          ++pos_p;
          ++pos_x;
        }

        return pos_p == end_p;
      }
    }

    bool starts_with (std::string const& p, std::string const& x)
    {
      return generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
    }

    bool ends_with (std::string const& s, std::string const& x)
    {
      return generic_starts_with (s.rbegin(), s.rend(), x.rbegin(), x.rend());
    }
  }
}
