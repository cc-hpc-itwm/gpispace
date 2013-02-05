// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_VALUE_HPP
#define _WE_TYPE_VALUE_CONTAINER_VALUE_HPP 1

#include <we/type/value/container/type.hpp>

namespace value
{
  namespace container
  {
    const value::type& find ( key_vec_t::const_iterator pos
                            , const key_vec_t::const_iterator end
                            , const value::type& store
                            );
    const value::type& value (const type&, const key_vec_t&);
  }
}

#endif
