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

#include <util-generic/testing/printer/chrono.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <functional>
#include <type_traits>

#define FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME_IMPL(timeout_)    \
  using BOOST_PP_CAT (futrmrt_Duration_, __LINE__)                      \
    = typename std::decay<decltype (timeout_)>::type;                   \
  struct                                                                \
  {                                                                     \
    BOOST_PP_CAT (futrmrt_Duration_, __LINE__) const allowed;           \
    void operator>> (std::function<void()> fun)                         \
    {                                                                   \
      auto const start (std::chrono::steady_clock::now());              \
      fun();                                                            \
      auto const end (std::chrono::steady_clock::now());                \
      BOOST_REQUIRE_LT (end - start, allowed);                          \
    }                                                                   \
  } BOOST_PP_CAT (futrmrt_, __LINE__) = {timeout_}; \
  BOOST_PP_CAT (futrmrt_, __LINE__) >> [&]
