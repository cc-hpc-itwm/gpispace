// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_PUT_HPP
#define _WE_TYPE_VALUE_PUT_HPP

#include <we/type/value.hpp>
#include <we/type/value/field.hpp>
#include <we/type/value/mk_structured.hpp>

#include <fhg/util/split.hpp>

namespace value
{
  inline void put ( path_type::const_iterator pos
                  , const path_type::const_iterator end
                  , type & store
                  , const type & value
                  )
  {
    if (pos == end)
      {
        store = value;
      }
    else
      {
        put ( pos + 1
            , end
            , field (*pos, value::mk_structured_or_keep (store))
            , value
            );
      }
  }

  inline void put ( const path_type & path
                  , type & store
                  , const type & value
                  )
  {
    put (path.begin(), path.end(), store, value);
  }

  inline void put ( const std::string & path
                         , type & store
                         , const type & value
                         )
  {
    put ( fhg::util::split<path_type::value_type, path_type> (path, '.')
        , store
        , value
        );
  }
}

#endif
