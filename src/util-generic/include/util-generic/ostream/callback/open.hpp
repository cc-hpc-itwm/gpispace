// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/callback/function.hpp>
#include <util-generic/ostream/callback/print.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        template<typename T, typename Open>
          constexpr print_function<T> open ( Open open
                                           , print_function<T> f = id<T>()
                                           )
        {
          return [f, open] (std::ostream& os, T const& x) -> std::ostream&
            {
              return os << open << print<decltype (x)> (f, x);
            };
        }
      }
    }
  }
}
