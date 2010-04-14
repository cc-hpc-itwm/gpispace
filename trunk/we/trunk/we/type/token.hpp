// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/expr/eval/context.hpp>

#include <we/type/control.hpp>
#include <we/type/id.hpp>
#include <we/type/literal.hpp>
#include <we/type/place.hpp>
#include <we/type/signature.hpp>

#include <we/util/show.hpp>

#include <string>
#include <stdexcept>

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
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

  struct structured_t;

  typedef expr::eval::context<signature::field_name_t> context_t;

  typedef boost::variant< control
                        , literal::type
                        , boost::recursive_wrapper<structured_t>
                        > value_t;

  struct structured_t
  {
  public:
    typedef boost::unordered_map< signature::field_name_t
                                , value_t
                                > map_t;
    typedef map_t::const_iterator const_iterator;
    typedef map_t::const_iterator iterator;

  private:
    map_t map;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(map);
    }
  public:
    value_t & operator [] (const signature::field_name_t & field_name)
    {
      return map[field_name];
    }

    const_iterator find (const signature::field_name_t & field_name) const
    {
      return map.find (field_name);
    }

    const_iterator begin (void) const { return map.begin(); }
    const_iterator end (void) const { return map.end(); }

    bool has_field (const signature::field_name_t & field_name) const
    {
      return map.find (field_name) != map.end();
    }
  };

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
    void operator () (const structured_t & map) const
    {
      for ( structured_t::const_iterator field (map.begin())
          ; field != map.end()
          ; ++field
          )
        boost::apply_visitor ( visitor_bind (name + "." + field->first, c)
                             , field->second
                             );
    }
  };

  class visitor_hash : public boost::static_visitor<std::size_t>
  {
  public:
    std::size_t operator () (const control &) const
    {
      return 42;
    }
    std::size_t operator () (const literal::type & v) const
    {
      return boost::hash_value(v);
    }

    std::size_t operator () (const structured_t & map) const
    {
      return boost::apply_visitor (visitor_hash(), (*(map.begin())).second);
    }
  };


  class visitor_show : public boost::static_visitor<std::string>
  {
  public:
    std::string operator () (const control & x) const
    {
      return util::show (x);
    }

    std::string operator () (const literal::type & v) const
    {
      return literal::show (v);
    }

    std::string operator () (const structured_t & map) const
    {
      std::string s;

      s += "[";

      for ( structured_t::const_iterator field (map.begin())
          ; field != map.end()
          ; ++field
          )
        s += ((field != map.begin()) ? ", " : "")
          +  field->first 
          +  " := "
          +  boost::apply_visitor (visitor_show(), field->second)
          ;

      s += "]";

      return s;
    }
  };

  class visitor_unbind : public boost::static_visitor<value_t>
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

    value_t operator () (const control &) const
    {
      return literal::require_type (field, "control", context.value (field));
    }

    value_t operator () (const signature::type_name_t & type_name) const
    {
      return literal::require_type (field, type_name, context.value (field));
    }

    value_t operator () (const signature::structured_t & signature) const
    {
      structured_t structured;

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

  // binary visiting
  class visitor_require_type : public boost::static_visitor<value_t>
  {
  private:
    const signature::field_name_t & field_name;

  public:
    visitor_require_type (const signature::field_name_t & _field_name)
      : field_name (_field_name)
    {}

    value_t operator () (const control &, const control & v) const
    {
      return v;
    }

    value_t operator () ( const signature::type_name_t & type_name
                        , const literal::type & v
                        ) const
    {
      return literal::require_type (field_name, type_name, v);
    }

    value_t operator () ( const signature::structured_t & signature
                        , const structured_t & token
                        ) const
    {
      for ( signature::structured_t::const_iterator sig (signature.begin())
          ; sig != signature.end()
          ; ++sig
          )
        {
          const structured_t::const_iterator pos (token.find (sig->first));

          if (!token.has_field (sig->first))
            throw std::runtime_error ("type error: missing field " + sig->first);

          boost::apply_visitor
            ( visitor_require_type (field_name + "." + sig->first)
            , sig->second
            , pos->second
            );
        }

      for ( structured_t::const_iterator field (token.begin())
          ; field != token.end()
          ; ++field
          )
        if (!signature.has_field (field->first))
          throw std::runtime_error ("type error: unknown field " + field->first);

      return token;
    }

    template<typename T, typename U>
    value_t operator () (const T &, const U &) const
    {
      throw std::runtime_error ("type error");
    }
  };

  class type
  {
  public:
    value_t value;

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
         , const value_t & v
         )
      : value ( boost::apply_visitor ( visitor_require_type (field)
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
    static const visitor_hash vh;

    return boost::apply_visitor (vh, t.value);
  }

  bool smaller_or_equal (const structured_t &, const structured_t &);

  class visitor_eq : public boost::static_visitor<bool>
  {
  public:
    bool operator () (const control &, const control &) const
    {
      return true;
    }

    bool operator () (const literal::type & x, const literal::type & y) const
    {
      return x == y;
    }

    bool operator () (const structured_t & x, const structured_t & y) const
    {
      return smaller_or_equal (x, y) && smaller_or_equal (y, x);
    }

    template<typename A, typename B>
    bool operator () (const A &, const B &) const
    {
      return false;
    }
  };

  bool smaller_or_equal (const structured_t & x, const structured_t & y)
  {
    bool all_eq (true);

    for ( structured_t::const_iterator field (x.begin())
        ; field != x.end() && all_eq == true
        ; ++field
        )
      {
        const structured_t::const_iterator pos (y.find (field->first));

        all_eq = (pos == y.end()) 
          ? false
          : boost::apply_visitor (visitor_eq(), field->second, pos->second);
      }

    return all_eq;
  }

  inline bool operator == (const type & a, const type & b)
  {
    return boost::apply_visitor (visitor_eq(), a.value, b.value);
  }

  inline bool operator != (const type & a, const type & b)
  {
    return !(a == b);
  }

  std::ostream & operator << (std::ostream & s, const type & t)
  {
    return s << boost::apply_visitor (visitor_show(), t.value);
  }

  template<typename NET>
  bool put ( NET & net
           , const petri_net::pid_t & pid
           , const value_t & v = control()
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
