// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
          ::boost::hash_combine (seed, std::get<index> (tuple));
        }
      };

    template <class tuple_type>
      struct hash_value_impl<tuple_type, 0>
    {
      static void apply (size_t& seed, tuple_type const& tuple)
      {
        ::boost::hash_combine (seed, std::get<0> (tuple));
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
