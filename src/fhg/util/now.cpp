#include <fhg/util/now.hpp>

#include <sys/time.h>

namespace fhg
{
  namespace util
  {
    double now ()
    {
      struct timeval tv;

      gettimeofday (&tv, nullptr);

      return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
    }
  }
}
