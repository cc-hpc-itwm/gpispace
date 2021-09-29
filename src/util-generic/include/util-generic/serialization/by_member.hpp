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

#include <util-generic/serialization/detail/base_class.hpp>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      //! Serialize a type_ by serializing it's (publically
      //! accessible) base_or_members_... individually.
      //! \note Requires that type_ is default constructible.
      //! \see base_class<T>()
      //! \todo Should we ensure that base classes are deserialized
      //! before the class itself?
#define FHG_UTIL_SERIALIZATION_BY_MEMBER(type_, base_or_members_...)    \
      FHG_UTIL_SERIALIZATION_DETAIL_BY_MEMBER (type_, base_or_members_)
    }
  }
}

#include <util-generic/serialization/by_member.ipp>
