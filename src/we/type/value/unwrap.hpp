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

#include <we/type/value.hpp>
#include <we/type/value/from_value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template <typename T>
      inline std::list<T> unwrap (std::list<value_type> const& lv)
      {
        std::list<T> lt;

        for (auto const& v : lv)
        {
          lt.emplace_back (from_value<T> (v));
        }

        return lt;
      }

      template <typename T>
      inline std::set<T> unwrap (std::set<value_type> const& sv)
      {
        std::set<T> st;

        for (auto const& v : sv)
        {
          st.emplace (from_value<T> (v));
        }

        return st;
      }

      template <typename K, typename V>
      inline std::map<K, V> unwrap (std::map<value_type, value_type> const& mvv)
      {
        std::map<K, V> mkv;

        for (auto const& kv : mvv)
        {
          mkv.emplace ( from_value<K> (kv.first)
                      , from_value<V> (kv.second)
                      );
        }

        return mkv;
      }
    }
  }
}
