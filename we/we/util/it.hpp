// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_IT_HPP
#define _UTIL_IT_HPP

#include <limits.h>
#include <cstddef>
#include <algorithm>

namespace it
{
  template<typename IT>
  class it
  {
  public:
    typedef std::size_t size_type;

  protected:
    IT pos;
    const IT end;
    typedef it<IT> super;

  private:
    const size_type the_size;
    const bool _empty;

  public:
    explicit it (const IT & _pos, const IT & _end)
      : pos (_pos)
      , end (_end)
      , the_size (std::distance (pos, end))
      , _empty (pos == end)
    {}

    bool has_more() const { return pos != end; }
    void operator++ () { ++pos; }

    size_type size (void) const { return the_size; }
    bool empty (void) const { return _empty; }
  };
}

#endif
