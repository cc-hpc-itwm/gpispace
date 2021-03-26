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

#include <util-generic/testing/random/impl.hpp>

#include <limits>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<typename T>
          using non_char_integral
            = std::integral_constant
                <bool, !std::is_same<T, char>{} && std::is_integral<T>{}>;

        template<typename T>
          struct random_impl
            <T, typename std::enable_if<non_char_integral<T>{}>::type>
        {
          struct non_zero{};

          //! Generate a random T between \a min and \a max (inclusive).
          T operator() ( T max = std::numeric_limits<T>::max()
                       , T min = std::numeric_limits<T>::lowest()
                       ) const;
          //! Generate a random non-zero T between \a min and \a max
          //! (inclusive).
          T operator() ( non_zero
                       , T max = std::numeric_limits<T>::max()
                       , T min = std::numeric_limits<T>::lowest()
                       ) const;
        };
      }

      //! \note deprecated Use random<T>{}().
      template<typename T>
        T random_integral();

      //! \note deprecated Use random<T>{} (random<T>::non-zero{}).
      template<typename T>
        T random_integral_without_zero();
    }
  }
}

#include <util-generic/testing/random/integral.ipp>
