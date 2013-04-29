// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_CPP_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_CPP_HPP

#include <we/type/signature.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace cpp
      {
        class header
        {
        public:
          header (const structured_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const structured_type& _structured;
        };
        std::ostream& operator<< (std::ostream&, const header&);
      }
    }
  }
}

#endif
