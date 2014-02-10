// mirko.rahn@itwm.fhg.de

#ifndef FHG_UTIL_LIFT_ERROR_CODE_TO_EXCEPTION_HPP
#define FHG_UTIL_LIFT_ERROR_CODE_TO_EXCEPTION_HPP

#include <string>

namespace fhg
{
  namespace util
  {
    void lift_error_code_to_exception (int ec, std::string const&);
  }
}

#endif
