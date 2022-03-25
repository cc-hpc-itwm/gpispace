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

namespace fhg
{
  namespace util
  {
    namespace
    {
      template<typename Head, typename... Tail>
        struct combined_hash_impl
      {
        static void apply (size_t& seed, Head const& head, Tail const&... tail)
        {
          combined_hash_impl<Head>::apply (seed, head);
          combined_hash_impl<Tail...>::apply (seed, tail...);
        }
      };

      template<typename Head>
        struct combined_hash_impl<Head>
      {
        static void apply (size_t& seed, Head const& head)
        {
          ::boost::hash_combine (seed, std::hash<Head>() (head));
        }
      };
    }

    template<typename... Values>
      std::size_t combined_hash (Values const&... values)
    {
      std::size_t seed (0);
      combined_hash_impl<Values...>::apply (seed, values...);
      return seed;
    }

#define FHG_UTIL_MAKE_COMBINED_STD_HASH(type_, varname_, ...)       \
    namespace std                                                   \
    {                                                               \
      template<> struct hash<type_>                                 \
      {                                                             \
        std::size_t operator() (type_ const& varname_) const        \
        {                                                           \
          return fhg::util::combined_hash (__VA_ARGS__);            \
        }                                                           \
      };                                                            \
    }
  }
}
