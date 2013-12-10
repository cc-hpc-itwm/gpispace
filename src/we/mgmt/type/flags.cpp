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
        os << (flags.canceling ? "C" : "c");
        os << (flags.canceled ? "T" : "t");
        os << (flags.failed ? "F" : "f");
        os << (flags.finished ? "D" : "d");
        return os;
      }

      bool is_alive (const flags_t& f)
      {
        return (  f.suspended
               || f.canceling
               || f.canceled
               || f.failed
               || f.finished
               ) == false;
      }

#define IMPL(_name)                                           \
      bool is_ ## _name (const flags_t& f)                    \
      {                                                       \
        return f._name;                                       \
      }                                                       \
      void set_ ## _name (flags_t& f, bool val)               \
      {                                                       \
        f._name = val;                                        \
      }

      IMPL(suspended);
      IMPL(canceling);
      IMPL(canceled);
      IMPL(failed);
      IMPL(finished);

#undef IMPL
    }
  }
}
