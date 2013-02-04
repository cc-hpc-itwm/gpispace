// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/put.hpp>
#include <we/type/value/field.hpp>
#include <we/type/value/mk_structured.hpp>

#include <fhg/util/split.hpp>

namespace value
{
  void put ( path_type::const_iterator pos
           , const path_type::const_iterator end
           , type& store
           , const type& value
           )
  {
    if (pos == end)
      {
        store = value;
      }
    else
      {
        const std::string& f (*pos); ++pos;

        put ( pos
            , end
            , field (f, value::mk_structured_or_keep (store))
            , value
            );
      }
  }

  void put (const path_type& path, type& store, const type& value)
  {
    put (path.begin(), path.end(), store, value);
  }

  void put (const std::string& path, type& store, const type& value)
  {
    put ( fhg::util::split<path_type::value_type, path_type> (path, '.')
        , store
        , value
        );
  }
}
