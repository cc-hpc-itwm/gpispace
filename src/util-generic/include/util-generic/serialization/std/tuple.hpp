// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <tuple>

namespace boost
{
  namespace serialization
  {
    namespace
    {
      template<std::size_t, std::size_t> struct serialize_impl;
      template<std::size_t N>
        struct serialize_impl<N, N>
      {
        template<typename Archive, typename... Elements>
          static Archive& apply (Archive& ar, std::tuple<Elements...> const&)
        {
          return ar;
        }
      };
      template<std::size_t Index, std::size_t Count>
        struct serialize_impl
      {
        template<typename Archive, typename... Elements>
          static Archive& apply (Archive& ar, std::tuple<Elements...> const& t)
        {
          ar & std::get<Index> (t);
          return serialize_impl<Index + 1, Count>::apply (ar, t);
        }
        template<typename Archive, typename... Elements>
          static Archive& apply (Archive& ar, std::tuple<Elements...>& t)
        {
          ar & std::get<Index> (t);
          return serialize_impl<Index + 1, Count>::apply (ar, t);
        }
      };
    }

    template<typename Archive, typename... Elements>
      Archive& serialize
        (Archive& ar, std::tuple<Elements...>& t, const unsigned int)
    {
      return serialize_impl<0, sizeof... (Elements)>::apply (ar, t);
    }
    template<typename Archive, typename... Elements>
      Archive& serialize
        (Archive& ar, std::tuple<Elements...> const& t, const unsigned int)
    {
      return serialize_impl<0, sizeof... (Elements)>::apply (ar, t);
    }
  }
}
