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

#include <boost/functional/hash.hpp>

#include <functional>
#include <tuple>

namespace std
{
  namespace
  {
    template < class tuple_type
             , size_t index = std::tuple_size<tuple_type>::value - 1
             >
      struct hash_value_impl
      {
        static void apply (size_t& seed, tuple_type const& tuple)
        {
          hash_value_impl<tuple_type, index - 1>::apply (seed, tuple);
          boost::hash_combine (seed, std::get<index> (tuple));
        }
      };

    template <class tuple_type>
      struct hash_value_impl<tuple_type, 0>
    {
      static void apply (size_t& seed, tuple_type const& tuple)
      {
        boost::hash_combine (seed, std::get<0> (tuple));
      }
    };
  }

  template<typename... Values>
    struct hash<std::tuple<Values...>>
  {
    std::size_t operator() (std::tuple<Values...> const& tuple) const
    {
      std::size_t seed (0);
      hash_value_impl<std::tuple<Values...>>::apply (seed, tuple);
      return seed;
    }
  };
}
