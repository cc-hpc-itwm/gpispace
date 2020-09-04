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
