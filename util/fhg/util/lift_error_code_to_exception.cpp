// mirko.rahn@itwm.fhg.de

#include <fhg/util/lift_error_code_to_exception.hpp>

#include <boost/format.hpp>

#include <stdexcept>

#include <cstring>
#include <cerrno>

namespace fhg
{
  namespace util
  {
    void lift_error_code_to_exception (int ec, std::string const& msg)
    {
      if (ec < 0)
      {
        int const err (errno);

        throw std::runtime_error
          ((boost::format ("%1% failed: %2%") % msg % strerror (err)).str());
      }
    }
  }
}
