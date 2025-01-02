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
      template<typename T>
        inline std::string to_string
          ( T x
          , callback::print_function<T> f = callback::id<T>()
          )
      {
        return callback::print<T> (f, x).string();
      }
    }
  }
}
