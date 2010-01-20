// mirko.rahn@itwm.fraunhofer.de

#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <limits.h>
#include <cstddef>
#include <algorithm>

namespace util
{
  template <class T>
  T next_power_of_two (T v)
  {
    if (v == 0)
      return 1;

    --v;

    for (std::size_t i (1); i < sizeof(T) * CHAR_BIT; i <<= 1)
      v |= (v >> i);

    return v + 1;
  }

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
