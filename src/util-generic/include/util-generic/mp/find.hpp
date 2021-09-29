// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <cstddef>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template <typename, std::size_t, typename...> struct find;
      }

      //! Returns an std::integral_constant<std::size_t> with the
      //! index of type T in types Types.
      //! \note un-instantiated if Types does not contain T
      //! \note returns the first position of T if Types contains T
      //! multiple times.
      template <typename T, typename... Types>
        using find = detail::find<T, 0, Types...>;
    }
  }
}

#include <util-generic/mp/find.ipp>
