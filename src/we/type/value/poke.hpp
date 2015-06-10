#pragma once

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      value_type& poke ( const std::list<std::string>::const_iterator&
                       , const std::list<std::string>::const_iterator&
                       , value_type&
                       , const value_type&
                       );
      value_type& poke ( const std::list<std::string>& path
                       , value_type& node
                       , const value_type& value
                       );
      value_type& poke (const std::string&, value_type&, const value_type&);
    }
  }
}
