// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/preprocessor/cat.hpp>

namespace fhg
{
  namespace util
  {
    //! \note variadic macros might not be implemented (or even
    //! implementable) for all sets of arguments due to the nature of
    //! how they have to be implemented.

    //! get nth variadic argument
#define FHG_UTIL_REQUIRED_VA_ARG(N, ...)                                       \
    BOOST_PP_CAT (FHG_UTIL_REQUIRED_VA_ARG_IMPL_, N) (__VA_ARGS__)

    //! get nth variadic argument or fallback-th if variadics too short
#define FHG_UTIL_OPTIONAL_VA_ARG(N, fallback /* <= N */, ...)                  \
    BOOST_PP_CAT (BOOST_PP_CAT (BOOST_PP_CAT (FHG_UTIL_OPTIONAL_VA_ARG_IMPL_, N), _), fallback) (__VA_ARGS__)

    namespace detail
    {
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_1_1(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (1, __VA_ARGS__)

#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_2_1(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (2, __VA_ARGS__, ##__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_2_2(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (2, __VA_ARGS__)

#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_3_1(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (3, __VA_ARGS__, ##__VA_ARGS__, ##__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_3_2(ig0, ...)                            \
      FHG_UTIL_OPTIONAL_VA_ARG_IMPL_2_1 (__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_3_3(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (3, __VA_ARGS__)

#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_4_2(ig0,  ...)                           \
      FHG_UTIL_OPTIONAL_VA_ARG_IMPL_3_1 (__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_4_3(ig0, ig1, ...)                       \
      FHG_UTIL_OPTIONAL_VA_ARG_IMPL_2_1 (__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_4_4(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (4, __VA_ARGS__)

#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_5_3(ig0, ig1,  ...)                      \
      FHG_UTIL_OPTIONAL_VA_ARG_IMPL_3_1 (__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_5_4(ig0, ig1, ig2, ...)                  \
      FHG_UTIL_OPTIONAL_VA_ARG_IMPL_2_1 (__VA_ARGS__)
#define FHG_UTIL_OPTIONAL_VA_ARG_IMPL_5_5(...)                                 \
      FHG_UTIL_REQUIRED_VA_ARG (5, __VA_ARGS__)

#define FHG_UTIL_REQUIRED_VA_ARG_IMPL_1(...) \
      FHG_UTIL_REQUIRED_VA_ARG_IMPL_2 (hack around expanding to , ##__VA_ARGS__)
#define FHG_UTIL_REQUIRED_VA_ARG_IMPL_2(ig0, wanted, ...) wanted
#define FHG_UTIL_REQUIRED_VA_ARG_IMPL_3(ig0, ig1, wanted, ...) wanted
#define FHG_UTIL_REQUIRED_VA_ARG_IMPL_4(ig0, ig1, ig2, wanted, ...) wanted
#define FHG_UTIL_REQUIRED_VA_ARG_IMPL_5(ig0, ig1, ig2, ig3, wanted, ...) wanted
    }
  }
}
