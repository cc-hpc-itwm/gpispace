// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SIGNATURE_HPP
#define PNET_SRC_WE_TYPE_VALUE_SIGNATURE_HPP

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      struct signature_type
      {
      public:
        signature_type ( const std::string& name
                       , const value_type& value = value_type()
                       )
          : _name (name)
          , _value (value)
        {}
        operator value_type&()
        {
          return _value;
        }
        const value_type& value() const
        {
          return _value;
        }

      private:
        std::string _name;
        value_type _value;
      };
    }
  }
}

#endif
