// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_GET_HPP
#define PNET_SRC_WE_TYPE_VALUE_GET_HPP

#include <we/type/value.hpp>

#include <boost/optional.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      //! \todo Avoid the copy, get a ref instead!?
      boost::optional<value_type> get ( const std::list<std::string>&
                                      , const value_type&
                                      );
    }
  }
}

#endif
