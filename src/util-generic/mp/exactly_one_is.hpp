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

#include <util-generic/mp/none_is.hpp>

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      //! Given a type Needle, checks that types Haystack contains
      //! exactly one Needle, i.e. not more than one or none.
      template<typename Needle, typename... Haystack>
        struct exactly_one_is : std::false_type {};
      template<typename T, typename... Tail>
        struct exactly_one_is<T, T, Tail...> : none_is<T, Tail...> {};
      template<typename T, typename Head, typename... Tail>
        struct exactly_one_is<T, Head, Tail...> : exactly_one_is<T, Tail...> {};
    }
  }
}
