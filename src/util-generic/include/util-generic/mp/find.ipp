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

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template <typename T, std::size_t N, typename... Tail>
          struct find<T, N, T, Tail...> : std::integral_constant<std::size_t, N> {};
        template <typename T, std::size_t N, typename Head, typename... Tail>
          struct find<T, N, Head, Tail...> : find<T, N + 1, Tail...> {};
      }
    }
  }
}
