// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
