#ifndef GWES_MEMORY_HPP
#define GWES_MEMORY_HPP 1

//#if HAVE_CONFIG_H
//#include <gwes/gwes-config.hpp>
//#endif

#define USE_STL_TR1 1

#if USE_STL_TR1 == 1
#include <tr1/memory>
#else
#include <boost/tr1/memory.hpp>
#endif

namespace gwes
{
    using ::std::tr1::shared_ptr;
}

#endif
