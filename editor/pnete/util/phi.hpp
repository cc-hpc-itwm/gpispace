// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_PHI_HPP
#define FHG_UTIL_PHI_HPP

#include <cmath>

namespace fhg
{
  namespace util
  {
    namespace phi
    {
      template<typename T>
        const T& value ()
      {
        static const T x ((1.0 + sqrt(5.0)) / 2.0);

        return x;
      }

      namespace ratio
      {
        template<typename T> T smaller (const T& x) { return x / value<T>(); }
        template<typename T> T bigger (const T& x) { return x * value<T>(); }
      }
    }
  }
}

#endif
