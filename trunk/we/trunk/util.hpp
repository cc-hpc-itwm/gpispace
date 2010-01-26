// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <limits.h>
#include <cstddef>
#include <algorithm>

namespace util
{
  template<typename IT>
  class it
  {
  protected:
    IT pos;
    const IT end;
  private:
    const std::size_t the_size;
  public:
    it (const IT & _pos, const IT & _end)
      : pos (_pos)
      , end (_end)
      , the_size (std::distance (pos, end))
    {}

    bool has_more (void) const { return (pos != end) ? true : false; }
    void operator ++ (void) { ++pos; }

    std::size_t size (void) const { return the_size; }
  };
}

#endif
