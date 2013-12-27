// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_LOG_LOGLEVEL_INC
#define FHG_LOG_LOGLEVEL_INC

#include <string>

namespace fhg
{
  namespace log
  {
    enum Level { TRACE
               , DEBUG
               , INFO
               , WARN
               , ERROR
               , FATAL
               };

    Level from_int (int);
    Level from_string (std::string const&);
    const std::string& string (Level);
  }
}

#endif
