// mirko.rahn@itwm.fraundhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_UTIL_LITERAL_HPP
#define PNET_SRC_WE_TYPE_VALUE_UTIL_LITERAL_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      typedef boost::variant< we::type::literal::control
                            , bool
                            , int
                            , long
                            , unsigned int
                            , unsigned long
                            , float
                            , double
                            , char
                            , std::string
                            , bitsetofint::type
                            , bytearray::type
                            > value_literal_type;
    }
  }
}

#endif
