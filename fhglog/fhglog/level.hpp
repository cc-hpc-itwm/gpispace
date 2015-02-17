// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <string>

namespace fhg
{
  namespace log
  {
    enum Level { TRACE
               , INFO
               , WARN
               , ERROR
               };

    Level from_string (std::string const&);
    const std::string& string (Level);
  }
}
