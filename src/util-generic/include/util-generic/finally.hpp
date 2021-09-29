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
        : _valid (true)
        , _function (std::forward<Function> (function))
      {}
      ~finally_t()
      {
        if (_valid)
        {
          _function();
        }
      }

      finally_t (finally_t&& other)
        : _valid (true)
        , _function (std::move (other._function))
      {
        other._valid = false;
      }

      finally_t& operator= (finally_t&&) = delete;
      finally_t (finally_t const&) = delete;
      finally_t& operator= (finally_t const&) = delete;

    private:
      bool _valid;
      Function _function;
    };

    template<typename Function, bool>
      finally_t<Function> finally (Function&& function)
    {
      return finally_t<Function> (std::forward<Function> (function));
    }
  }
}
