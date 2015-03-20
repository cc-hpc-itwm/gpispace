// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <we/type/signature.hpp>

#include <util-generic/ostream_modifier.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      class show : public fhg::util::ostream::modifier
      {
      public:
        show (const signature_type&);
        virtual std::ostream& operator() (std::ostream&) const override;
      private:
        const signature_type& _signature;
      };
    }
  }
}
