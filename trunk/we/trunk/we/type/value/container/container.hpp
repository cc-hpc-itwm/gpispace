// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_CONTAINER_HPP
#define _WE_TYPE_VALUE_CONTAINER_CONTAINER_HPP 1

#include <we/type/value/container/type.hpp>
#include <we/type/value/container/exception.hpp>

namespace value
{
  namespace container
  {
    inline value::type bind ( type & container
                            , const std::string & key
                            , const value::type & value
                            )
    {
      container[key] = value;

      return value;
    }

    inline const value::type & value ( const type & container
                                     , const std::string & key
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

#endif
