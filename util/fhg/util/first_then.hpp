// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_FIRST_THEN_HPP
#define FHG_UTIL_FIRST_THEN_HPP

#include <fhg/util/ostream_modifier.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    template<typename T>
      class first_then : public ostream::modifier
    {
    public:
      first_then (const T& f, const T& t)
        : _first (f)
        , _then (t)
      {}
      std::ostream& operator() (std::ostream& os) const
      {
        os << _first;
        _first = _then;
        return os;
      }
    private:
      mutable T _first;
      const T _then;
    };
  }
}

#endif
