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

#include <utility>

namespace fhg
{
  namespace util
  {
    template<typename T>
      struct temporary_return_value_holder
    {
      template<typename Fun, typename... Args>
        temporary_return_value_holder (Fun&& fun, Args&&... args)
          : _value (fun (std::forward<Args> (args)...))
      {}
      T operator*() const { return std::move (_value); }

    private:
      T _value;
    };

    template<typename T>
      struct temporary_return_value_holder<T&>
    {
      template<typename Fun, typename... Args>
        temporary_return_value_holder (Fun&& fun, Args&&... args)
          : _value (fun (std::forward<Args> (args)...))
      {}
      T& operator*() const { return _value; }

    private:
      T& _value;
    };

    template<>
      struct temporary_return_value_holder<void>
    {
      template<typename Fun, typename... Args>
        temporary_return_value_holder (Fun&& fun, Args&&... args)
      {
        fun (std::forward<Args> (args)...);
      }
      void operator*() const {}
    };
  }
}
