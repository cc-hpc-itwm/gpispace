// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <iosfwd>

namespace we
{
  namespace type
  {
    namespace literal
    {
      struct control
      {
        friend std::ostream& operator<< (std::ostream&, const control&);
        friend bool operator== (const control&, const control&);

        friend std::size_t hash_value (const control&);
        friend bool operator< (const control&, const control&);
      };
    }
  }
}

//! \todo REMOVE! This is deprecated but some clients still use it.
typedef we::type::literal::control control;
