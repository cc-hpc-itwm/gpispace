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

#include <we/type/value.hpp>
#include <we/type/value/to_value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T>
        inline std::list<value_type>
        wrap (std::list<T> const& lT)
      {
        std::list<value_type> lv;

        for (auto const& x : lT)
        {
          lv.emplace_back (to_value (x));
        }

        return lv;
      }

      template<>
        inline std::list<value_type>
        wrap<value_type> (std::list<value_type> const& lv)
      {
        return lv;
      }

      template<typename T>
        inline std::set<value_type>
        wrap (std::set<T> const& sT)
      {
        std::set<value_type> sv;

        for (auto const& x : sT)
        {
          sv.emplace (to_value (x));
        }

        return sv;
      }

      template<typename K, typename V>
        inline std::map<value_type, value_type>
        wrap (std::map<K, V> const& mkv)
      {
        std::map<value_type, value_type> mvv;

        for (auto const& kv : mkv)
        {
          mvv.emplace ( to_value (kv.first)
                      , to_value (kv.second)
                      );
        }

        return mvv;
      }

      template<>
        inline std::map<value_type, value_type>
        wrap<value_type, value_type> (std::map< value_type
                                              , value_type
                                              > const& mvv
                                     )
      {
        return mvv;
      }

      template<>
        inline std::set<value_type>
        wrap<value_type> (std::set<value_type> const& lv)
      {
        return lv;
      }
    }
  }
}
