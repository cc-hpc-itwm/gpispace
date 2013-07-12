// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_POKE_HPP
#define PNET_SRC_WE_TYPE_VALUE_POKE_HPP

#include <we2/type/value.hpp>

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

#endif
