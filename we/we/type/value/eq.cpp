// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/eq.hpp>

#include <we/type/literal.hpp>

namespace value
{
  bool eq (const type& x, const type& y)
  {
    return x == y;
  }
}
