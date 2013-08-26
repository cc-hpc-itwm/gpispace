// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_READ_HPP
#define PNET_SRC_WE_TYPE_VALUE_READ_HPP

#include <we/type/value.hpp>

#include <fhg/util/parse/position.hpp>

#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      value_type read (fhg::util::parse::position&);
      value_type read (const std::string&);
    }
  }
}

#endif
