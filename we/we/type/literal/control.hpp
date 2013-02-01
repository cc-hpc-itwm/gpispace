// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONTROL_HPP
#define _WE_TYPE_CONTROL_HPP

#include <iosfwd>

#include <boost/serialization/nvp.hpp>

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

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& , const unsigned int)
        {
        }
      };
    }
  }
}

//! \todo REMOVE! This is deprecated but some clients still use it.
typedef we::type::literal::control control;

#endif
