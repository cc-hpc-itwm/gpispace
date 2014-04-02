// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_MACROS_HPP
#define FHG_UTIL_MACROS_HPP

//! \note A function that never returns, i.e. always throws or calls exit()
#if defined (__GNUC__) || defined (__clang__)
#define FHG_ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#if __cplusplus >= 201103L
#define FHG_ATTRIBUTE_NORETURN [[noreturn]]
#endif
#define FHG_ATTRIBUTE_NORETURN
#endif

#ifndef NDEBUG
#define IFNDEF_NDEBUG(x...) x
#else
#define IFNDEF_NDEBUG(x...)
#endif

#include <stdexcept>
#include <boost/format.hpp>
#include <boost/current_function.hpp>
#define INVALID_ENUM_VALUE(type, value) \
  throw std::out_of_range \
     ((boost::format \
      ("%1%:%2%: %3%: value of enum '%4%' expected: got non-enummed-value %5%") \
      % __FILE__ % __LINE__ % BOOST_CURRENT_FUNCTION % #type % value \
      ).str() \
     );

#endif
