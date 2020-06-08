#pragma once

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

      void dump_to (fhg::util::xml::xmlstream&, const structured_type&);
    }
  }
}
