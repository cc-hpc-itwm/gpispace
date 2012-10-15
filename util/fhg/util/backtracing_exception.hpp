// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_UTIL_BACKTRACING_EXCEPTION_HPP
#define _FHG_UTIL_BACKTRACING_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    class backtracing_exception : public std::runtime_error
    {
    public:
      backtracing_exception (const std::string& reason);
    };
  }
}

#endif
