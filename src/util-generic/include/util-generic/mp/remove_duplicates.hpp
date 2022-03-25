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

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      namespace detail
      {
        template<typename> struct remove_duplicates;
      }

      //! Given a type-sequence Sequence = sequence_types<Types...>,
      //! returns sequence_type<unique (Types...)>.
      //! \note Is O(Types^2), so use with care.
      template<typename Sequence>
        using remove_duplicates = typename detail::remove_duplicates<Sequence>::type;
    }
  }
}

#include <util-generic/mp/remove_duplicates.ipp>
