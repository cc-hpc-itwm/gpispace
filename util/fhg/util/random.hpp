#ifndef FHG_UTIL_RANDOM_HPP
#define FHG_UTIL_RANDOM_HPP

#include <stdlib.h>
#include <fhg/assert.hpp>

namespace fhg
{
  namespace util
  {
    namespace random
    {
      inline int rand_in (int a, int b)
      {
        if (b < a)
        {
          int tmp = a;
          a = b;
          b = tmp;
        }

        assert (b > a);

        int r = rand ();

        return (r % (b-a) + a);
      }
    }
  }
}

#endif
