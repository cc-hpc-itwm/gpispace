#ifndef FHG_ASSERT_HPP
#define FHG_ASSERT_HPP

/**
   - fhg_assert is a macro that provides a better mechanism than the
   legacy 'assert' and can be switched on/off independently

   - supports different modes:

   FHG_ASSERT_DISABLED    0
   make fhg_assert a nop

   FHG_ASSERT_ENABLED   1
   log to std::cerr and abort()

   FHG_ASSERT_LEGACY    2
   redirect to legacy assert()

   FHG_ASSERT_EXCEPTION 3
   throw an fhg::assertion_failed exception

   FHG_ASSERT_LOG       4
   just log to std::cerr

   FHG_ASSERT_LOG_ABORT 5
   just log to std::cerr

   - the syntax is quite easy:

   fhg_assert(<condition>, [message])

   the message is optional but if given, must be a C-string
 */

#include <sstream>
#include <fhg/assert_modes.hpp>
#include <fhg/assert_helper.hpp>

#define FHG_ASSERT_STR_(x) #x
#define FHG_ASSERT_STR(x) FHG_ASSERT_STR_(x)

#ifndef FHG_ASSERT_MODE
#  define FHG_ASSERT_MODE FHG_ASSERT_DISABLED
#endif

#if   FHG_ASSERT_DISABLED == FHG_ASSERT_MODE

#  define fhg_assert(cond, ...)

#elif FHG_ASSERT_ENABLED == FHG_ASSERT_MODE
#  include <stdlib.h>
#  include <iostream>

#  define fhg_assert(cond, ...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      std::cerr << fhg::assert_helper::message                          \
                   (FHG_ASSERT_STR(cond),  "" __VA_ARGS__, __FILE__, __LINE__)  \
                << std::endl                                            \
                << std::flush;                                          \
      abort();                                                          \
    }                                                                   \
  } while (0)

#elif FHG_ASSERT_LEGACY == FHG_ASSERT_MODE
#  include <cassert>
#  define fhg_assert(cond, ...) assert(cond)

#elif FHG_ASSERT_EXCEPTION == FHG_ASSERT_MODE
#  include <fhg/assertion_failed.hpp>
#  define fhg_assert(cond, ...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      throw fhg::assertion_failed                                       \
            (FHG_ASSERT_STR(cond), "" __VA_ARGS__, __FILE__, __LINE__);         \
    }                                                                   \
  } while (0)

#elif FHG_ASSERT_LOG == FHG_ASSERT_MODE
#  include <fhglog/minimal.hpp>
#  define fhg_assert(cond, ...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      LOG(ERROR, fhg::assert_helper::message                            \
         (FHG_ASSERT_STR(cond),  "" __VA_ARGS__, __FILE__, __LINE__)            \
         );                                                             \
    }                                                                   \
  } while (0)

#elif FHG_ASSERT_LOG_ABORT == FHG_ASSERT_MODE
#  include <fhglog/minimal.hpp>
#  define fhg_assert(cond, ...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      LOG(ERROR, fhg::assert_helper::message                            \
                 (FHG_ASSERT_STR(cond), "" __VA_ARGS__, __FILE__, __LINE__)     \
         );                                                             \
      abort();                                                          \
    }                                                                   \
  } while (0)

#else
#  error invalid FHG_ASSERT_MODE
#endif

#ifdef FHG_ASSERT_REPLACE_LEGACY
#  if FHG_ASSERT_LEGACY != FHG_ASSERT_MODE
#    undef assert
#    define assert(cond) fhg_assert(cond, #cond)
#  else
#    error cannot replace legacy 'assert' statements in FHG_ASSERT_LEGACY mode!
#  endif

#else
#  ifndef assert
#    define assert(cond)
#  endif
#endif

#endif
