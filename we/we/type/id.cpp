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
#define INVALID2(_prefix)                                               \
  const _prefix ## _type& _prefix ## _invalid()                         \
  {                                                                     \
    static const _prefix ## _type v                                     \
      (std::numeric_limits<_prefix ## _type::pod_type>::max());         \
                                                                        \
    return v;                                                           \
  }

  INVALID (eid);
  INVALID2 (place_id);

  INVALID2 (priority);

#undef INVALID
#undef INVALID2
}
