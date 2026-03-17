// Copyright (C) 2016,2019-2020,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/testing/random/impl.hpp>

#include <type_traits>



    namespace gspc::testing
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



#include <gspc/testing/random/floating_point.ipp>
