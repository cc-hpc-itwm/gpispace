// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/type/literal/control.hpp>

#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <we/type/value/require_type.hpp>
#include <we/type/value/show.hpp>

#include <boost/serialization/nvp.hpp>

#include <iostream>

namespace token
{
  class type
  {
  public:
    type ()
      : _value (we::type::literal::control())
    {}
    type (const value::type& value)
      : _value (value)
    {}

    // construct from value, require type from signature
    type ( const signature::field_name_t& field
         , const signature::type& signature
         , const value::type& value
         )
      : _value (value::require_type (field, signature, value))
    {}

    const value::type& value() const { return _value; }

  private:
    value::type _value;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP (_value);
    }
  };

  inline bool operator== (const type& a, const type& b)
  {
    return a.value() == b.value();
  }

  inline bool operator!= (const type& a, const type& b)
  {
    return !(a == b);
  }

  inline std::ostream& operator<< (std::ostream& s, const type& t)
  {
    return s << t.value();
  }
}

#endif
