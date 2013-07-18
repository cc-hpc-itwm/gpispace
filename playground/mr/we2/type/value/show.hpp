// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_SHOW_HPP
#define PNET_SRC_WE_TYPE_VALUE_SHOW_HPP

#include <we2/type/value.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class show
      {
      public:
        show (const value_type&);
        std::ostream& operator() (std::ostream&) const;
      private:
        const value_type& _value;
      };
      std::ostream& operator<< (std::ostream&, const show&);
    }
  }
}

#endif
