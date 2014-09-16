// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_MACROS_HPP
#define FHG_UTIL_MACROS_HPP

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

//! \todo more than just gcc?
#define UNREACHABLE()                           \
  __builtin_unreachable()

#endif
