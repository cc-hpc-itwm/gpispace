// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_LOG_LOGLEVEL_INC
#define FHG_LOG_LOGLEVEL_INC

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

#endif
