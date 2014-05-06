#ifndef FHG_ASSERT_HPP
#define FHG_ASSERT_HPP

#include <fhg/util/macros.hpp>

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
#define FHG_ASSERT_EXCEPTION 1 //! \note throw fhg::assertion_failed

#include <sstream>

#define FHG_ASSERT_STR_(x) #x
#define FHG_ASSERT_STR(x) FHG_ASSERT_STR_(x)

#ifndef FHG_ASSERT_MODE
#  define FHG_ASSERT_MODE FHG_ASSERT_DISABLED
#endif

#if   FHG_ASSERT_DISABLED == FHG_ASSERT_MODE

#  define IF_FHG_ASSERT(x...)
#  define fhg_assert(cond, ...)

#elif FHG_ASSERT_EXCEPTION == FHG_ASSERT_MODE
#  include <fhg/assertion_failed.hpp>
#  define IF_FHG_ASSERT(x...) x
#  define fhg_assert(cond, ...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      throw fhg::assertion_failed                                       \
            (FHG_ASSERT_STR(cond), "" __VA_ARGS__, __FILE__, __LINE__);         \
    }                                                                   \
  } while (0)

#else
#  error invalid FHG_ASSERT_MODE
#endif

#endif
