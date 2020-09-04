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

#if HAS_STD_GET_BY_TYPE

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      using std::get;
    }
  }
}

#else

#include <util-generic/mp/exactly_one_is.hpp>
#include <util-generic/mp/find.hpp>

//! \note n3404: Tuple Tidbits

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      template <typename T, typename... Elems>
        constexpr typename std::enable_if<mp::exactly_one_is<T, Elems...>::value, T const&>::type
          get (std::tuple<Elems...> const& tuple)
      {
        return std::get<mp::find<T, Elems...>::value> (tuple);
      }

      template <typename T, typename... Elems>
        constexpr typename std::enable_if<mp::exactly_one_is<T, Elems...>::value, T&>::type
          get (std::tuple<Elems...>& tuple)
      {
        return std::get<mp::find<T, Elems...>::value> (tuple);
      }
    }
  }
}

#endif
