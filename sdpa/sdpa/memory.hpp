#ifndef SDPA_MEMORY_HPP
#define SDPA_MEMORY_HPP 1

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

#if defined(USE_STL_TR1) && (USE_STL_TR1 == 1)
#include <tr1/memory>
#else
#include <boost/tr1/memory.hpp>
#endif

namespace sdpa {
    using std::tr1::shared_ptr;
    using std::tr1::weak_ptr;
  using std::tr1::enable_shared_from_this;
}

#endif
