// mirko.rahn@itwm.fraunhofer.de

#include <we/type/id.hpp>

#include <limits>

namespace petri_net
{
#define INVALID(_type)                                                  \
  const _type ## _t& _type ## _invalid()                                \
  {                                                                     \
    static const _type ## _t v                                          \
      (std::numeric_limits<_type ## _t::pod_type>::max());              \
                                                                        \
    return v;                                                           \
  }

  INVALID (eid);
  INVALID (pid);
  INVALID (prio);

#undef INVALID
}
