#include "util.hpp"
#include <sys/time.h>

long long sdpa::util::now() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (tv.tv_sec * 1000000 + tv.tv_usec);
}

/*
long long sdpa::util::timediff(const struct timeval &start, const struct timeval &end) {
  return (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
}
*/
