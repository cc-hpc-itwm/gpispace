#pragma once

#include <we/type/value.hpp>

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
