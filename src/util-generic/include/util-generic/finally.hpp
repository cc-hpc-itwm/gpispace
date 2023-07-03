// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/callable_signature.hpp>

#include <boost/preprocessor/cat.hpp>

#include <utility>

namespace fhg
{
  namespace util
  {
    template<typename Function> struct finally_t;
    template<typename Function, bool = is_callable<Function, void()>::value>
      finally_t<Function> finally (Function&&);

#define FHG_UTIL_FINALLY \
    auto const BOOST_PP_CAT (_finally_, __LINE__) = ::fhg::util::finally


    template<typename Function>
      struct finally_t
    {
      finally_t (Function&& function)
        : _function (std::forward<Function> (function))
      {}
      ~finally_t()
      {
        if (_valid)
        {
          _function();
        }
      }

      finally_t (finally_t&& other) noexcept
        : _valid (true)
        , _function (std::move (other._function))
      {
        other._valid = false;
      }

      finally_t& operator= (finally_t&&) = delete;
      finally_t (finally_t const&) = delete;
      finally_t& operator= (finally_t const&) = delete;

    private:
      bool _valid {true};
      Function _function;
    };

    template<typename Function, bool>
      finally_t<Function> finally (Function&& function)
    {
      return finally_t<Function> (std::forward<Function> (function));
    }
  }
}
