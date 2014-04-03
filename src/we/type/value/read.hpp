// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_READ_HPP
#define PNET_SRC_WE_TYPE_VALUE_READ_HPP

#include <we/type/value.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      class position;
    }
  }
}

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
