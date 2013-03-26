// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/find.hpp>

#include <we/type/value/get.hpp>

namespace value
{
  const value::type& find ( std::list<std::string>::const_iterator pos
                          , const std::list<std::string>::const_iterator end
                          , const value::type& store
                          )
  {
    if (pos == end)
    {
      return store;
    }
    else
    {
      const std::string& field (*pos); ++pos;

      return value::find (pos, end, value::get_field (field, store));
    }
  }
}
