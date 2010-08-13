// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_PUT_HPP
#define _WE_TYPE_VALUE_PUT_HPP

#include <we/type/value.hpp>
#include <we/type/value/field.hpp>
#include <we/type/value/mk_structured.hpp>

#include <fhg/util/split.hpp>

namespace value
{
  inline type put ( path_type::const_iterator pos
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
        store = boost::apply_visitor (visitor::mk_structured(), store);

        put ( pos + 1
            , end
            , boost::apply_visitor (visitor::field (*pos), store)
            , value
            );
      }

    return value;
  }

  inline type put ( const path_type & path
                  , type & store
                  , const type & value
                  )
  {
    return put (path.begin(), path.end(), store, value);
  }

  inline type put ( const std::string & path
                  , type & store
                  , const type & value
                  )
  {
    return put ( fhg::util::split<path_type::value_type, path_type> (path, '.')
               , store
               , value
               );
  }
}

#endif
