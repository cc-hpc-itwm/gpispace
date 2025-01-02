// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>

#include <cstdint>
#include <functional>

namespace std
{
  namespace
  {
    struct hash_value_visitor : private ::boost::static_visitor<std::size_t>
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
