#ifndef SEDA_SHARED_PTR_HPP
#define SEDA_SHARED_PTR_HPP 1

#define USE_STL_TR1 1

#if USE_STL_TR1 == 1
#include <tr1/memory>
#else
#include <boost/tr1/memory.hpp>
#endif


namespace seda {
    using std::tr1::shared_ptr;
}

#endif
