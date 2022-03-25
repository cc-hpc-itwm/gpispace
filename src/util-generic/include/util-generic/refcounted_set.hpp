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

#pragma once

#include <set>
#include <unordered_map>

namespace fhg
{
  namespace util
  {
    template<typename T>
      class refcounted_set
    {
    public:
      void emplace (T x)
      {
        ++_count[x];
        _xs.emplace (std::move (x));
      }
      void erase (T const& x)
      {
        if (--_count.at (x) == 0)
        {
          _xs.erase (x);
          _count.erase (x);
        }
      }
      bool contains (T const& x) const
      {
        return _xs.count (x);
      }
      operator std::set<T> const&() const
      {
        return _xs;
      }

    private:
      std::set<T> _xs;
      std::unordered_map<T, std::size_t> _count;
    };
  }
}
