// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_READ_HPP
#define _WE_TYPE_VALUE_READ_HPP 1

#include <we/type/value.hpp>

#include <fhg/util/parse/position.hpp>

#include <string>

namespace value
{
  std::string identifier (fhg::util::parse::position&);

  type read (fhg::util::parse::position&);
  type read (const std::string&);
}

#endif
