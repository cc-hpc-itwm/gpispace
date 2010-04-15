// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/expr/eval/context.hpp>

#include <we/type/literal.hpp>
#include <we/type/place.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <we/util/show.hpp>

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

  typedef expr::eval::context<signature::field_name_t> context_t;

  class visitor_bind : public boost::static_visitor<>
  {
  private:
    const signature::field_name_t & name;
    context_t & c;
  public:
    visitor_bind (const signature::field_name_t & _name, context_t & _c)
      : name (_name)
      , c (_c)
    {}

    void operator () (const control &) const { c.bind (name, control()); }
    void operator () (const literal::type & v) const { c.bind (name, v); }
    void operator () (const value::structured_t & map) const
    {
      for ( value::structured_t::const_iterator field (map.begin())
          ; field != map.end()
          ; ++field
          )
        boost::apply_visitor ( visitor_bind (name + "." + field->first, c)
                             , field->second
                             );
    }
  };

  class visitor_unbind : public boost::static_visitor<value::type>
  {
  private:
    const signature::field_name_t & field;
    const context_t & context;

  public:
    visitor_unbind ( const signature::field_name_t & _field
                   , const context_t & _context
                   )
      : field (_field)
      , context (_context)
    {}

    value::type operator () (const control &) const
    {
      return literal::require_type (field, "control", context.value (field));
    }

    value::type operator () (const signature::type_name_t & type_name) const
    {
      return literal::require_type (field, type_name, context.value (field));
    }

    value::type operator () (const signature::structured_t & signature) const
    {
      value::structured_t structured;

      for ( signature::structured_t::const_iterator sig (signature.begin())
          ; sig != signature.end()
          ; ++sig
          )
        structured[sig->first] = 
          boost::apply_visitor 
           ( visitor_unbind (field + "." + sig->first, context)
           , sig->second
           );

      return structured;
    }
  };

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
    type () : value (control()) {}

    // construct from value, require type from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const value::type & v
         )
      : value ( boost::apply_visitor ( value::visitor_require_type (field)
                                     , signature.desc()
                                     , v
                                     )
              )
    {}

    // construct from context, use information from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const context_t & context
         )
      : value (boost::apply_visitor ( visitor_unbind (field, context)
                                    , signature.desc()
                                    )
              )
    {}
      
    void bind (const signature::field_name_t & field, context_t & c) const
    {
      boost::apply_visitor (visitor_bind (field, c), value);
    }

    friend std::ostream & operator << (std::ostream &, const type &);
    friend bool operator == (const type &, const type &);
    friend bool operator != (const type &, const type &);
    friend std::size_t hash_value (const type &);
  };

  inline std::size_t hash_value (const type & t)
  {
    return boost::apply_visitor (value::visitor_hash(), t.value);
  }

  inline bool operator == (const type & a, const type & b)
  {
    return boost::apply_visitor (value::visitor_eq(), a.value, b.value);
  }

  inline bool operator != (const type & a, const type & b)
  {
    return !(a == b);
  }

  std::ostream & operator << (std::ostream & s, const type & t)
  {
    return boost::apply_visitor (value::visitor_show (s), t.value);
  }

  template<typename NET>
  bool put ( NET & net
           , const petri_net::pid_t & pid
           , const value::type & v = control()
           )
  {
    return net.put_token ( pid
                         , type ( place::name<NET> (net, pid)
                                , place::signature<NET> (net, pid)
                                , v
                                )
                         );
  }
}

#endif
