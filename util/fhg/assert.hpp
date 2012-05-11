#ifndef FHG_ASSERT_HPP
#define FHG_ASSERT_HPP

/**
   - fhg_assert is a macro that provides a better mechanism than the
   legacy 'assert' and can be switched on/off independently

   - supports different modes:

   FHG_ASSERT_IGNORE    0
   make fhg_assert a nop

   FHG_ASSERT_ENABLED   1
   log to std::cerr and abort()

   FHG_ASSERT_LEGACY    2
   redirect to legacy assert()

   FHG_ASSERT_EXCEPTION 3
   throw an fhg::assertion_failed exception

   FHG_ASSERT_LOG       4
   just log to std::cerr

   - the syntax is quite easy:

   fhg_assert(<condition>, [message])

   the message is optional but if given, must be a C-string
 */

#include <sstream>

namespace fhg
{
  class assertion_failed : public std::exception
  {
  public:
    assertion_failed ( std::string const & cond
                     , std::string const & message
                     , std::string const & file
                     , int line
                     )
      : m_cond (cond)
      , m_message (message)
      , m_file (file)
      , m_line (line)
    {
      std::ostringstream sstr;
      sstr << "assertion '" << m_cond << "'"
           << " in " << m_file << ":" << m_line
           << " failed: " << m_message;
      m_what_text = sstr.str();
    }

    virtual ~assertion_failed() throw() {}

    const char * what () const throw() { return m_what_text.c_str(); }

    std::string const & condition() const { return m_cond; }
    std::string const & message() const { return m_message; }
    std::string const & file() const { return m_file; }
    int line() const { return m_line; }
  private:
    std::string m_cond;
    std::string m_message;
    std::string m_file;
    int         m_line;
    std::string m_what_text;
  };
}

#define FHG_ASSERT_IGNORE    0
#define FHG_ASSERT_ENABLED   1
#define FHG_ASSERT_LEGACY    2
#define FHG_ASSERT_EXCEPTION 3
#define FHG_ASSERT_LOG       4
//#define FHG_ASSERT_LOG_ABORT 5

#define FHG_ASSERT_STR_(x) #x
#define FHG_ASSERT_STR(x) FHG_ASSERT_STR_(x)

#ifndef FHG_ASSERT_MODE
#  define FHG_ASSERT_MODE FHG_ASSERT_IGNORE
#endif

#if   FHG_ASSERT_IGNORE == FHG_ASSERT_MODE

#  define fhg_assert(cond, msg)

#elif FHG_ASSERT_ENABLED == FHG_ASSERT_MODE
#  include <stdlib.h>
#  include <iostream>

#  define fhg_assert(cond, msg...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      std::cerr << "*** assertion '" << FHG_ASSERT_STR(cond) << "'"     \
                << " in " << __FILE__ << ":" << __LINE__                \
                << " failed: " << "" msg                                \
                << std::endl                                            \
                << std::flush;                                          \
      abort();                                                          \
    }                                                                   \
  } while (0)

#elif FHG_ASSERT_LEGACY == FHG_ASSERT_MODE
#  include <cassert>
#  define fhg_assert(cond, msg...) assert(cond)

#elif FHG_ASSERT_EXCEPTION == FHG_ASSERT_MODE
#  define fhg_assert(cond, msg...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      throw fhg::assertion_failed ( FHG_ASSERT_STR(cond)                \
                                  , "" msg, __FILE__, __LINE__);        \
    }                                                                   \
  } while (0)

#elif FHG_ASSERT_LOG == FHG_ASSERT_MODE
#  include <iostream>
#  define fhg_assert(cond, msg...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      std::cerr << "*** assertion '" << FHG_ASSERT_STR(cond) << "'"     \
                << " in " << __FILE__ << ":" << __LINE__                \
      ;                                                                 \
      if (! #msg[0])                                                    \
      {                                                                 \
        std::cerr << " failed";                                         \
      }                                                                 \
      else                                                              \
      {                                                                 \
        std::cerr << " failed: " << "" msg;                             \
      }                                                                 \
      std::cerr << std::endl << std::flush;                             \
    }                                                                   \
  } while (0)

#if 0

#elif FHG_ASSERT_LOG_ABORT == FHG_ASSERT_MODE
#  include <fhglog/minimal.hpp>
#  define fhg_assert(cond, msg...)                                      \
  do                                                                    \
  {                                                                     \
    if (! (cond))                                                       \
    {                                                                   \
      LOG(ERROR, "*** assertion '" << FHG_ASSERT_STR(cond) << "'"       \
         << " in " << __FILE__ << ":" << __LINE__                       \
         << " failed: " << msg << std::flush                            \
         );                                                             \
      abort();                                                          \
    }                                                                   \
  } while (0)

#endif // if 0

#endif

#ifdef FHG_ASSERT_REPLACE_LEGACY

#  if FHG_ASSERT_LEGACY != FHG_ASSERT_MODE
#    undef assert
#    define assert(cond) fhg_assert(cond)
#  else
#    error cannot replace legacy 'assert' statements in FHG_ASSERT_LEGACY mode!
#  endif

#else
#  ifndef assert
#    define assert(cond)
#  endif
#endif

#endif
