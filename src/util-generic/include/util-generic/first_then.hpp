// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/ostream/modifier.hpp>

#include <functional>
#include <iostream>

namespace fhg
{
  namespace util
  {
    template<typename T>
      class first_then : public ostream::modifier
    {
    public:
      first_then (const T& f, const T& t)
        : _value (f)
        , _modify (std::bind (&first_then::set, this, t))
      {}
      std::ostream& operator() (std::ostream& os) const override
      {
        os << _value;
        _modify();
        return os;
      }

    private:
      mutable T _value;
      mutable std::function<void ()> _modify;

      void set (const T& value) const
      {
        _value = value;
        _modify = &first_then::nop;
      }
      static void nop() {}
    };
  }
}
