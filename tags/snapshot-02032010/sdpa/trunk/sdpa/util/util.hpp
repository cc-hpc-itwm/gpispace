#ifndef SDPA_UTIL_HPP
#define SDPA_UTIL_HPP 1

#include <sdpa/types.hpp>

namespace sdpa { namespace util {
  typedef unsigned long long time_type;

  /**
    Returns the current time in microseconds.

    on Unix this is essentially:
       struct timeval tv;
       gettimeofday(&tv, NULL);
       return (tv.tv_sec * 1000000 + tv.tv_usec);
  */
  extern time_type now();
  extern time_type time_diff(const time_type &start, const time_type &end);
}}

#endif
