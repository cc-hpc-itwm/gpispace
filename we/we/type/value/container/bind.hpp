// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_BIND_HPP
#define _WE_TYPE_VALUE_CONTAINER_BIND_HPP 1

#include <we/type/value/container/type.hpp>

namespace value
{
  namespace container
  {
    void bind ( type&
              , const std::string&
              , const std::list<std::string>&
              , const value::type&
              );
    void bind ( type&
              , const std::string&
              , const std::string&
              , const value::type&
              );
    void bind ( type&
              , const key_vec_t&
              , const value::type&
              );
  }
}

#endif
