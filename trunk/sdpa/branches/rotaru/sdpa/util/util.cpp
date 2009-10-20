#include "util.hpp"
#include <sys/time.h>

sdpa::util::time_type sdpa::util::now() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return sdpa::util::time_type(tv.tv_sec * 1000000 + tv.tv_usec);
}

sdpa::util::time_type sdpa::util::time_diff(const sdpa::util::time_type &start, const sdpa::util::time_type &end) {
  return end - start;
}

/*
long long sdpa::util::timediff(const struct timeval &start, const struct timeval &end) {
  return (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
}
*/
