// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
