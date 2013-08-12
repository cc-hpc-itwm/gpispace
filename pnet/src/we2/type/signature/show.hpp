// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_SHOW_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_SHOW_HPP

#include <we2/type/signature.hpp>

#include <fhg/util/ostream_modifier.hpp>

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
        std::ostream& operator() (std::ostream&) const;
      private:
        const signature_type& _signature;
      };
    }
  }
}

#endif
