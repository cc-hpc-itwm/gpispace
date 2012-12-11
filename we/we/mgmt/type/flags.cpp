// mirko.rahn@itwm.fhg.de

#include <we/mgmt/type/flags.hpp>

#include <ostream>

namespace we
{
  namespace mgmt
  {
    namespace flags
    {
      std::ostream& operator<< (std::ostream& os, const flags_t& flags)
      {
        os << (flags.suspended ? "S" : "s");
        os << (flags.cancelling ? "C" : "c");
        os << (flags.cancelled ? "T" : "t");
        os << (flags.failed ? "F" : "f");
        os << (flags.finished ? "D" : "d");
        return os;
      }
    }
  }
}
