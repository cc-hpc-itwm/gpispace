// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_IT_HPP
#define _UTIL_IT_HPP

#include <limits.h>
#include <cstddef>
#include <algorithm>

namespace it
{
  template<typename IT, typename SizeType=std::size_t>
  class it
  {
  public:
	typedef SizeType size_type;

  protected:
    IT pos;
    const IT end;
    typedef it<IT> super;
  private:
    const size_type the_size;
  public:
    explicit it (const IT & _pos, const IT & _end)
      : pos (_pos)
      , end (_end)
      , the_size (std::distance (pos, end))
    {}

    bool has_more (void) const { return (pos != end) ? true : false; }
    void operator ++ (void) { ++pos; }

    size_type size (void) const { return the_size; }
    bool empty (void) const { return the_size == 0; }
  };
}

#endif
