// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_PROPERTY_HPP
#define _XML_PARSE_UTIL_PROPERTY_HPP

#include <xml/parse/state.hpp>

#include <we/type/property.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        void set ( const state::type & state
                 , we::type::property::type & prop
                 , const we::type::property::path_type & path
                 , const we::type::property::value_type & value
                 );
        void set_state ( state::type& state
                       , we::type::property::type& prop
                       , const we::type::property::path_type& path
                       , const we::type::property::value_type& value
                       = we::type::property::value_type()
                       );
        void join ( const state::type& state
                  , we::type::property::type& x
                  , const we::type::property::type& y
                  );
      }
    }
  }
}

#endif
