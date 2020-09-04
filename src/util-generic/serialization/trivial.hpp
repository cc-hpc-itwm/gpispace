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

#include <util-generic/cxx17/logical_operator_type_traits.hpp>
#include <util-generic/serialization/detail/base_class.hpp>

#include <boost/serialization/is_bitwise_serializable.hpp>

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace serialization
    {
      //! Check if all given Types are trivially serializable.
      template<typename... Types>
        using is_trivially_serializable
          = cxx17::conjunction
              <boost::serialization::is_bitwise_serializable<Types>...>;

      //! Let the serialization of type_ with the
      //! base_or_members_... be trivial. base_or_members_... are
      //! asserted to be trivially serializable.
      //! \see base_class<T>()
#define FHG_UTIL_SERIALIZATION_TRIVIAL(type_, base_or_members_...)      \
      FHG_UTIL_SERIALIZATION_DETAIL_TRIVIAL (type_, base_or_members_)
    }
  }
}

#include <util-generic/serialization/trivial.ipp>
