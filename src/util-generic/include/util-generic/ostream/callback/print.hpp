// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/ostream/callback/function.hpp>
#include <util-generic/ostream/modifier.hpp>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        template<typename T>
          class print : public modifier
        {
        public:
          print (print_function<T> print_, T x)
            : _print (print_)
            , _x (x)
          {}
          std::ostream& operator() (std::ostream& os) const override
          {
            _print (os, _x); return os;
          }
        private:
          print_function<T> const _print;
          T const _x;
        };
      }
    }
  }
}
