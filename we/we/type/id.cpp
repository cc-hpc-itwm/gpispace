// mirko.rahn@itwm.fraunhofer.de

#include <we/type/id.hpp>

#include <limits>

namespace petri_net
{
#define INVALID(_prefix)                                                \
  const _prefix ## _type& _prefix ## _invalid()                         \
  {                                                                     \
    static const _prefix ## _type v                                     \
      (std::numeric_limits<_prefix ## _type::pod_type>::max());         \
                                                                        \
    return v;                                                           \
  }

  INVALID (edge_id);
  INVALID (place_id);
  INVALID (activity_id);

  INVALID (priority);

#undef INVALID

#define GENERATE(_prefix)                               \
  _prefix ## _type _prefix ## _generate()               \
  {                                                     \
    static _prefix ## _type v (0);                      \
                                                        \
    return v++;                                         \
  }

  GENERATE (activity_id);
#undef GENERATE
}
