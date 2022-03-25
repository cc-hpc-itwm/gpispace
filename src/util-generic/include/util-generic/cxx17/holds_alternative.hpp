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

#include<boost/variant/variant_fwd.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      /**
       * @brief holds_alternative implementation for ::boost::variant
       *
       * This is an implementation of holds_alternative for ::boost::variant.
       * It checks if the given ::boost::variant holds the queried alternative
       * type.
       * The call is ill-formed if the queried type doesn't appear exactly once
       * in the variant's types.
       *
       * @tparam T The type to check within the Variant
       * @tparam Ts Types contained within the ::boost::variant variable
       * @param[in] variant ::boost::variant object to query
       * @return True if T is found exactly once, False otherwise.
       */
      template<typename T, typename... Ts>
        bool holds_alternative (::boost::variant<Ts...> const& variant) noexcept;
    }
  }
}

#include <util-generic/cxx17/holds_alternative.ipp>
