#ifndef GWDL_MEMORY_H
#define GWDL_MEMORY_H 1

//#if HAVE_CONFIG_H
//#include <gwdl/gwdl-config.hpp>
//#endif

#define USE_STL_TR1 1

#if USE_STL_TR1 == 1
#include <tr1/memory>
#else
#include <boost/tr1/memory.hpp>
#endif

namespace gwdl
{
    using std::tr1::shared_ptr;
}

#endif
