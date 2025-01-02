// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
