// mirko.rahn@itwm.fraunhofer.de

#include <we/type/id.hpp>

#include <limits>

namespace petri_net
{
  std::size_t hash_value (const pid_t& pid)
  {
    return boost::hash<uint64_t>() (pid._value);
  }

  std::ostream& operator<< (std::ostream& s, const pid_t& pid)
  {
    return s << pid._value;
  }
  std::istream& operator>> (std::istream& i, pid_t& pid)
  {
    return i >> pid._value;
  }

#define INVALID(_type)                                                  \
  const _type ## _t& _type ## _invalid()                                \
  {                                                                     \
    static const _type ## _t v                                          \
      (std::numeric_limits<_type ## _t>::max());                        \
                                                                        \
    return v;                                                           \
  }

  INVALID(eid);
  INVALID(pid);
  INVALID(prio);

#undef INVALID
}
