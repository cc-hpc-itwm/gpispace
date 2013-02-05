// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/expr/eval/context.hpp>

#include <we/type/literal.hpp>
#include <we/type/place.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>
#include <we/type/id.hpp>

#include <we/type/value/eq.hpp>
#include <we/type/value/require_type.hpp>
#include <we/type/value/show.hpp>

#include <string>
#include <stdexcept>

#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>

#include <iostream>

namespace token
{
  class type
  {
  public:
    value::type value;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(value);
    }

  public:
    type ()
      : value (we::type::literal::control())
    {}

    // construct from value, require type from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const value::type & v
         )
      : value (value::require_type (field, signature, v))
    {}

    // construct from context, use information from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const expr::eval::context & context
         )
      : value (value::require_type (field, signature, context.value (field)))
    {}

    friend inline std::ostream & operator << (std::ostream &, const type &);
    friend bool operator == (const type &, const type &);
    friend bool operator != (const type &, const type &);
  };

  inline bool operator == (const type & a, const type & b)
  {
    return value::eq (a.value, b.value);
  }

  inline bool operator != (const type & a, const type & b)
  {
    return !(a == b);
  }

  inline std::ostream & operator << (std::ostream & s, const type & t)
  {
    return s << t.value;
  }
}

#endif
