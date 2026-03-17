// Copyright (C) 2012-2015,2021-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
   - gspc_assert is a macro that provides a better mechanism than the
   legacy 'assert' and can be switched on/off independently

   - supports different modes:

   GSPC_ASSERT_DISABLED    0
   make gspc_assert a nop

   GSPC_ASSERT_EXCEPTION 1
   throw an std::logic_error exception

   - the syntax is quite easy:

   gspc_assert(<condition>, [message])

   the message is optional but if given, must be a C-string
 */
#define GSPC_ASSERT_DISABLED  0 //! \note ignore asserts
#define GSPC_ASSERT_EXCEPTION 1 //! \note throw std::logic_error

#ifndef GSPC_ASSERT_MODE
#  error missing definition for GSPC_ASSERT_MODE
#endif

#if   GSPC_ASSERT_DISABLED == GSPC_ASSERT_MODE

#  define IF_GSPC_ASSERT(x...)
#  define gspc_assert(cond, ...)

#elif GSPC_ASSERT_EXCEPTION == GSPC_ASSERT_MODE
#  include <fmt/core.h>
#  include <stdexcept>
#  include <string>
#  define GSPC_ASSERT_STR_(x) #x
#  define GSPC_ASSERT_STR(x) GSPC_ASSERT_STR_(x)

#  define IF_GSPC_ASSERT(x...) x
#  define gspc_assert(cond, ...)                                        \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      throw std::logic_error                                            \
        ( fmt::format                                                   \
          ( "[{0}:{1}] assertion '{2}' failed{3}{4}."                   \
          , __FILE__                                                    \
          , __LINE__                                                    \
          , GSPC_ASSERT_STR (cond)                                      \
          , (std::string ("" __VA_ARGS__).empty() ? "" : ": ")          \
          , std::string ("" __VA_ARGS__)                                \
          )                                                             \
        );                                                              \
    }                                                                   \
  } while (0)

#else
#  error invalid GSPC_ASSERT_MODE
#endif
