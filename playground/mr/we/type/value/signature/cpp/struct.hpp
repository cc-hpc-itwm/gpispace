// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_VALUE_SIGNATURE_CPP_STRUCT_HPP
#define WE_TYPE_VALUE_SIGNATURE_CPP_STRUCT_HPP

#include <we/type/value/signature.hpp>

#include <fhg/util/indenter.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class as_struct
      {
      public:
        as_struct (const signature_type&, fhg::util::indenter&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const signature_type& _signature;
        fhg::util::indenter& _indent;
      };
      std::ostream& operator<< (std::ostream&, const as_struct&);
    }
  }
}

#endif
