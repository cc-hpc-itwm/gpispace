// Copyright (C) 2023 Fraunhofer ITWM
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
        template<typename T, typename Close>
          constexpr print_function<T> close ( Close close
                                            , print_function<T> f = id<T>()
                                            )
        {
          return [f, close] (std::ostream& os, T const& x) -> std::ostream&
            {
              return os << print<decltype (x)> (f, x) << close;
            };
        }
      }
    }
  }
}
