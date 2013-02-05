// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/container.hpp>

#include <we/type/value/show.hpp>

#include <iostream>

namespace value
{
  namespace container
  {
    value::type bind ( type& container
                     , const std::string& key
                     , const value::type& value
                     )
    {
      container[key] = value;

      return value;
    }

    const value::type& value ( const type& container
                             , const std::string& key
                             )
    {
      const type::const_iterator pos (container.find (key));

      if (pos == container.end())
      {
        throw exception::missing_binding (key);
      }
      else
      {
        return pos->second;
      }
    }
  }
}

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
