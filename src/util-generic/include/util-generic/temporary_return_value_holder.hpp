// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
