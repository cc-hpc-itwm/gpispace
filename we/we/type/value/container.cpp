// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/container.hpp>

#include <we/type/value/show.hpp>

#include <iostream>

namespace std
{
  std::ostream& operator<< (std::ostream& s, const value::container::type& t)
  {
    for ( value::container::type::const_iterator pos (t.begin())
        ; pos != t.end()
        ; ++pos
        )
    {
      s << pos->first << " := " << pos->second << std::endl;
    }
    return s;
  }
}
