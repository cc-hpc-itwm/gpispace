#ifndef SDPA_UTIL_HPP
#define SDPA_UTIL_HPP 1

#include <fhg/util/now.hpp>

namespace sdpa { namespace util {
  typedef double time_type;

  /**
    Returns the current time in microseconds.

    on Unix this is essentially:
       struct timeval tv;
       gettimeofday(&tv, NULL);
       return (tv.tv_sec * 1000000 + tv.tv_usec);
  */
    using fhg::util::now;
}}

#endif
