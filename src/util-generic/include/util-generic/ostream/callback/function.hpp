// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <iosfwd>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        template<typename T>
          using print_function
            = std::function<std::ostream& (std::ostream&, T const&)>;

        template<typename T, typename U>
          class generic
        {
        public:
          generic (std::function<U (T const&)> const& transform)
            : _transform (transform)
          {}
          std::ostream& operator() (std::ostream& os, T const& x) const
          {
            return os << _transform (x);
          }

        private:
          std::function<U (T const&)> const _transform;
        };

        template<typename T>
          constexpr generic<T, T const&> id()
        {
          return generic<T, T const&>
            {[] (T const& x) -> T const& { return x; }};
        }

        template<typename T, typename U>
          constexpr generic<T, U> select
            (std::function<U (T const* const)> select)
        {
          return generic<T, U>
            {[select] (T const& x) { return select (&x); }};
        }
      }
    }
  }
}
