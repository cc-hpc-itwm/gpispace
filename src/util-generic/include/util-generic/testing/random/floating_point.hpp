// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/testing/random/impl.hpp>

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
          struct random_impl
            <T, typename std::enable_if<std::is_floating_point<T>{}>::type>
        {
          T operator()() const;
        };
      }

      //! \note: deprecated Use random<T>{}() instead.
      template<typename T>
        T random_floating_point();
    }
  }
}

#include <util-generic/testing/random/floating_point.ipp>
