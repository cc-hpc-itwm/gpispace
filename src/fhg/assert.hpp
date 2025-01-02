// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
   - fhg_assert is a macro that provides a better mechanism than the
   legacy 'assert' and can be switched on/off independently

   - supports different modes:

   FHG_ASSERT_DISABLED    0
   make fhg_assert a nop

   FHG_ASSERT_EXCEPTION 1
   throw an fhg::assertion_failed exception

   - the syntax is quite easy:

   fhg_assert(<condition>, [message])

   the message is optional but if given, must be a C-string
 */
#define FHG_ASSERT_DISABLED  0 //! \note ignore asserts
#define FHG_ASSERT_EXCEPTION 1 //! \note throw std::logic_error

#ifndef FHG_ASSERT_MODE
#  error missing definition for FHG_ASSERT_MODE
#endif

#if   FHG_ASSERT_DISABLED == FHG_ASSERT_MODE

#  define IF_FHG_ASSERT(x...)
#  define fhg_assert(cond, ...)

#elif FHG_ASSERT_EXCEPTION == FHG_ASSERT_MODE
#  include <fmt/core.h>
#  include <stdexcept>
#  include <string>
#  define FHG_ASSERT_STR_(x) #x
#  define FHG_ASSERT_STR(x) FHG_ASSERT_STR_(x)

#  define IF_FHG_ASSERT(x...) x
#  define fhg_assert(cond, ...)                                         \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      throw std::logic_error                                            \
        ( fmt::format                                                   \
          ( "[{0}:{1}] assertion '{2}' failed{3}{4}."                   \
          , __FILE__                                                    \
          , __LINE__                                                    \
          , FHG_ASSERT_STR (cond)                                       \
          , (std::string ("" __VA_ARGS__).empty() ? "" : ": ")          \
          , std::string ("" __VA_ARGS__)                                \
          )                                                             \
        );                                                              \
    }                                                                   \
  } while (0)

#else
#  error invalid FHG_ASSERT_MODE
#endif
