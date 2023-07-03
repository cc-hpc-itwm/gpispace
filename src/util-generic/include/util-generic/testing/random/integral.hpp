// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
