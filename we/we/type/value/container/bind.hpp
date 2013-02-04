// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_BIND_HPP
#define _WE_TYPE_VALUE_CONTAINER_BIND_HPP 1

#include <we/type/value/container/type.hpp>

#include <we/type/value/put.hpp>

#include <we/type/value/mk_structured.hpp>

namespace value
{
  namespace container
  {
    template<typename Path>
    inline void bind ( type & container
                     , const std::string & key
                     , const Path & path
                     , const value::type & value
                     )
    {
      value::put ( path
                 , value::mk_structured_or_keep (container[key])
                 , value
                 );
    }

    inline void bind ( type & container
                     , const key_vec_t & key_vec
                     , const value::type & value
                     )
    {
      if (key_vec.size() == 0)
        {
          throw std::runtime_error ("value::container::bind []");
        }

      key_vec_t::const_iterator pos (key_vec.begin());
      const std::string& key (*pos); ++pos;

      value::put
        ( pos
        , key_vec.end()
        , value::mk_structured_or_keep (container[key])
        , value
        );
    }
  }
}

#endif
