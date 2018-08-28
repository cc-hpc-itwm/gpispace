#pragma once

#include <boost/current_function.hpp>
#include <boost/format.hpp>

#include <stdexcept>
#include <type_traits>

#define INVALID_ENUM_VALUE(type_, value_)                               \
  throw std::out_of_range                                               \
    ( ( boost::format                                                   \
          ( "%1%:%2%: %3%: value of enum '%4%' expected: "              \
            "got non-enummed-value %5%"                                 \
          )                                                             \
      % __FILE__                                                        \
      % __LINE__                                                        \
      % BOOST_CURRENT_FUNCTION                                          \
      % #type_                                                          \
      % static_cast<std::underlying_type<type_>::type> (value_)         \
      ).str()                                                           \
    )
