// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_REMOVE_HPP
#define PNET_SRC_WE_TYPE_VALUE_REMOVE_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      void remove ( const std::list<std::string>::const_iterator&
                  , const std::list<std::string>::const_iterator&
                  , value_type&
                  );
      void remove ( const std::list<std::string>& path
                  , value_type& node
                  );
      void remove (const std::string&, value_type&);
    }
  }
}

#endif
