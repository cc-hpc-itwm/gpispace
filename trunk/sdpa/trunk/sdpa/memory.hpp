#ifndef SDPA_MEMORY_HPP
#define SDPA_MEMORY_HPP 1

#if HAVE_CONFIG_H
#include <sdpa/sdpa-config.h>
#endif

#if USE_STL_TR1
#include <tr1/memory>
#else
#include <boost/tr1/memory.hpp>
#endif

namespace sdpa {
    using std::tr1::shared_ptr;
}

#endif
