// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SHOW_HPP
#define PNET_SRC_WE_TYPE_VALUE_SHOW_HPP

#include <we2/type/value.hpp>

#include <fhg/util/ostream_modifier.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class show : public fhg::util::ostream::modifier
      {
      public:
        show (const value_type&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const value_type& _value;
      };
    }
  }
}

#endif
