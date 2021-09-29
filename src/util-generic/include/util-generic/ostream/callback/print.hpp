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
          virtual std::ostream& operator() (std::ostream& os) const override
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
