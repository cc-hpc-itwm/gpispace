// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_DUMP_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_DUMP_HPP

#include <we/type/signature.hpp>

#include <fhg/util/xml.fwd.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      class dump
      {
      public:
        dump (const structured_type&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const structured_type& _structured;
      };
      std::ostream& operator<< (std::ostream&, const dump&);
    }
  }
}

#endif
