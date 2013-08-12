// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_SIGNATURE_CPP_HPP
#define PNET_SRC_WE_TYPE_SIGNATURE_CPP_HPP

#include <we2/type/signature.hpp>

#include <iosfwd>
#include <list>
#include <string>

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

        class header_signature
        {
        public:
          header_signature (const signature_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const signature_type& _signature;
        };
        std::ostream& operator<< (std::ostream&, const header_signature&);

        class impl
        {
        public:
          impl (const structured_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const structured_type& _structured;
        };
        std::ostream& operator<< (std::ostream&, const impl&);

        class impl_signature
        {
        public:
          impl_signature (const signature_type&);
          std::ostream& operator() (std::ostream&) const;
        private:
          const signature_type& _signature;
        };
        std::ostream& operator<< (std::ostream&, const impl_signature&);
      }
    }
  }
}

#endif
