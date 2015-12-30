// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <fhglog/level.hpp>

#include <string>

namespace fhg
{
  namespace log
  {
    Level from_string (std::string const&);
    const std::string& string (Level);
  }
}
