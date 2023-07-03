// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/callback/close.hpp>
#include <util-generic/ostream/callback/open.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        template<typename T, typename Bracket>
          constexpr print_function<T> bracket ( Bracket o
                                              , Bracket c
                                              , print_function<T> f = id<T>()
                                              )
        {
          return [f, o, c] (std::ostream& os, T const& x) -> std::ostream&
            {
              return open (o, close (c, f)) (os, x);
            };
        }
      }
    }
  }
}
