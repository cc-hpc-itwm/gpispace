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

  namespace visitor
  {
    class bind : public boost::static_visitor<>
    {
    private:
      const signature::field_name_t & name;
      context_t & c;
    public:
      bind (const signature::field_name_t & _name, context_t & _c)
        : name (_name)
        , c (_c)
      {}

      void operator () (const literal::type & v) const { c.bind (name, v); }
      void operator () (const value::structured_t & map) const
      {
        c.bind (name, map);

        for ( value::structured_t::const_iterator field (map.begin())
            ; field != map.end()
            ; ++field
            )
          boost::apply_visitor ( bind (name + "." + field->first, c)
                               , field->second
                               );
      }
    };

    class unbind : public boost::static_visitor<>
    {
    private:
      const signature::field_name_t & field;
      const context_t & context;
      value::type & val;

    public:
      unbind ( const signature::field_name_t & _field
             , const context_t & _context
             , value::type & _val
             )
        : field (_field)
        , context (_context)
        , val (_val)
      {}

      void operator () (const literal::type_name_t & type_name) const
      {
        val = value::require_type (field, type_name, context.value (field));
      }

      void operator () (const signature::structured_t & signature) const
      {
        // first extract the complete value
        try
          {
            signature::type sig (signature);

            val = boost::apply_visitor ( value::visitor::require_type (field)
                                       , sig.desc()
                                       , context.value (field)
                                       );
          }
        catch (expr::exception::eval::missing_binding<signature::field_name_t>)
          {
            // there is no complete value given, just adjust the type

            if (!boost::apply_visitor(value::visitor::is_structured(), val))
               val = value::structured_t();
          }

        // special settings for the components are prefered
        for ( signature::structured_t::const_iterator sig (signature.begin())
            ; sig != signature.end()
            ; ++sig
            )
          if (boost::apply_visitor ( value::visitor::has_field (sig->first)
                                   , val
                                   )
             )
            { // the field is already there
              // get a reference to the subvalue
              value::type & subval
                ( boost::apply_visitor ( value::visitor::field (sig->first)
                                       , val
                                       )
                );

              try
                {
                  // try to set the subval
                  boost::apply_visitor 
                    ( unbind (field + "." + sig->first, context, subval)
                    , sig->second
                    );
                }
              catch (expr::exception::eval::missing_binding<signature::field_name_t>)
                {
                  // do nothing, the field was already there
                }
            }
          else
            { // the field is not set already
              // get a reference to the subvalue, default construct it
              value::type & subval
                ( boost::apply_visitor ( value::visitor::field (sig->first)
                                       , val
                                       )
                );

              // try to set the subval, no catch!
              boost::apply_visitor 
                ( unbind (field + "." + sig->first, context, subval)
                , sig->second
                );
            }
      }
    };
  }

  class type
  {
  public:
    value::type value;
    std::size_t hash;

    friend class boost::serialization::access;
    template<typename Archive>
    void save (Archive & ar, const unsigned int) const
    {
      ar & BOOST_SERIALIZATION_NVP(value);
    }
    template<typename Archive>
    void load (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(value);
      hash = boost::apply_visitor (value::visitor::hash(), value);
    }
    
    BOOST_SERIALIZATION_SPLIT_MEMBER()

  public:
    type () 
      : value (control())
      , hash (boost::apply_visitor (value::visitor::hash(), value))
    {}

    // construct from value, require type from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const value::type & v
         )
      : value ( boost::apply_visitor ( value::visitor::require_type (field)
                                     , signature.desc()
                                     , v
                                     )
              )
      , hash (boost::apply_visitor (value::visitor::hash(), value))
    {}

    // construct from context, use information from signature
    type ( const signature::field_name_t & field
         , const signature::type & signature
         , const context_t & context
         )
    {
      boost::apply_visitor ( visitor::unbind (field, context, value)
                           , signature.desc()
                           );

      hash = boost::apply_visitor (value::visitor::hash(), value);
    }
      
    void bind (const signature::field_name_t & field, context_t & c) const
    {
      boost::apply_visitor (visitor::bind (field, c), value);
    }

    friend std::ostream & operator << (std::ostream &, const type &);
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
    return 
      a.hash == b.hash
      &&
      boost::apply_visitor (value::visitor::eq(), a.value, b.value);
  }

  inline bool operator != (const type & a, const type & b)
  {
    return !(a == b);
  }

  std::ostream & operator << (std::ostream & s, const type & t)
  {
    return s << t.value;
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

  template<typename NET>
  bool put ( NET & net
           , const petri_net::pid_t & pid
           , const type & t
           )
  {
    return put (net, pid, t.value);
  }
}

#endif
