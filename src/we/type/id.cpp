// mirko.rahn@itwm.fraunhofer.de

#include <we/type/id.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

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

  INVALID (place_id)
  INVALID (transition_id)

  INVALID (priority)

#undef INVALID
}
