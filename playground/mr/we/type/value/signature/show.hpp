// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SIGNATURE_SHOW_HPP
#define PNET_SRC_WE_TYPE_VALUE_SIGNATURE_SHOW_HPP

#include <we/type/value/signature.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class as_signature
      {
      public:
        as_signature (const value_type&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const value_type& _value;
      };
      std::ostream& operator<< (std::ostream&, const as_signature&);

      std::ostream& operator<< (std::ostream&, const signature_type&);
    }
  }
}

#endif
