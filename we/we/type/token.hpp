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
#include <we/type/value/hash.hpp>
#include <we/type/value/require_type.hpp>
#include <we/type/value/show.hpp>

#include <string>
#include <stdexcept>

#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>

#include <iostream>

namespace token
{
  namespace exception
  {
    class unknown_field : public std::runtime_error
    {
    public:
      unknown_field (const std::string & what) : std::runtime_error (what) {}
    };
  }

  typedef expr::eval::context context_t;

  class type
  {
  public:
    value::type value;
    std::size_t hash;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(value);
      ar & BOOST_SERIALIZATION_NVP(hash);
    }

  public:
    type ()
      : value (we::type::literal::control())
      , hash (value::hash_value (value))
    {}

    // construct from value, require type from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const value::type & v
         )
      : value (value::require_type (field, signature, v))
      , hash (value::hash_value (value))
    {}

    // construct from context, use information from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const context_t & context
         )
      : value (value::require_type (field, signature, context.value (field)))
      , hash (value::hash_value (value))
    {}

    friend inline std::ostream & operator << (std::ostream &, const type &);
    friend bool operator == (const type &, const type &);
    friend bool operator != (const type &, const type &);
    friend std::size_t hash_value (const type &);
  };

  inline std::size_t hash_value (const type & t)
  {
    return t.hash;
  }

  inline bool operator == (const type & a, const type & b)
  {
    return a.hash == b.hash && value::eq (a.value, b.value);
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
