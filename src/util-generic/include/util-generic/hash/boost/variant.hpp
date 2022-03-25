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

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>

#include <cstdint>
#include <functional>

namespace std
{
  namespace
  {
    struct hash_value_visitor : ::boost::static_visitor<std::size_t>
    {
      template<typename T>
        std::size_t operator() (T const& x) const
      {
        return std::hash<T>{} (x);
      }
    };
  }

  template<typename... Values>
    struct hash<::boost::variant<Values...>>
  {
    std::size_t operator() (::boost::variant<Values...> const& variant) const
    {
      std::size_t seed (0);
      ::boost::hash_combine (seed, variant.which());
      ::boost::hash_combine
        (seed, ::boost::apply_visitor (hash_value_visitor{}, variant));
      return seed;
    }
  };
}
