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

#include <functional>

#if ! HAS_STD_HASH_FOR_ENUM

#include <type_traits>

//! \note LWG defect 2148: Hashing enums should be supported directly
//! by std::hash

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      //! \note based on libstdcxx implementation in bits/functional_hash.h
      template<typename Enum, bool = std::is_enum<Enum>::value>
        struct hash_enum
      {
        hash_enum() = delete;
      };

      template<typename Enum>
        struct hash_enum<Enum, true>
      {
        std::size_t operator() (Enum value) const noexcept
        {
          using underlying = typename std::underlying_type<Enum>::type;
          return std::hash<underlying>{} (static_cast<underlying> (value));
        }
      };
    }
  }
}


namespace std
{
  template<typename Enum> struct hash : fhg::util::detail::hash_enum<Enum> {};
}

#endif
