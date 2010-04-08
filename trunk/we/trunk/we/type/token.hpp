// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/expr/variant/variant.hpp>
#include <we/expr/eval/context.hpp>

#include <we/type/control.hpp>
#include <we/type/signature.hpp>

#include <we/util/show.hpp>

#include <string>
#include <stdexcept>

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>

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

  typedef boost::unordered_map< signature::field_name_t
                              , expr::variant::type
                              > structured_t;

  typedef expr::eval::context<signature::field_name_t> context_t;

  typedef boost::variant< control
                        , expr::variant::type
                        , structured_t
                        > value_t;

  class visitor_bind : public boost::static_visitor<>
  {
  private:
    const signature::field_name_t & pref;
    context_t & c;
  public:
    visitor_bind (const signature::field_name_t & _pref, context_t & _c)
      : pref (_pref)
      , c (_c)
    {}

    void operator () (const control &) const { c.bind (pref, control()); }
    void operator () (const expr::variant::type & v) const { c.bind (pref, v); }
    void operator () (const structured_t & map) const
    {
      for ( structured_t::const_iterator field (map.begin())
          ; field != map.end()
          ; ++field
          )
        c.bind (pref + "." + field->first, field->second);
    }
  };

  class visitor_hash : public boost::static_visitor<std::size_t>
  {
  public:
    std::size_t operator () (const control &) const
    {
      return 42;
    }
    std::size_t operator () (const expr::variant::type & v) const
    {
      return boost::hash_value(v);
    }

    std::size_t operator () (const structured_t & map) const
    {
      return boost::hash_value ((*(map.begin())).second);
    }
  };


  class visitor_show : public boost::static_visitor<std::string>
  {
  public:
    std::string operator () (const control & x) const
    {
      return util::show (x);
    }

    std::string operator () (const expr::variant::type & v) const
    {
      return expr::variant::show (v);
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
          +  expr::variant::show (field->second)
          ;

      s += "]";

      return s;
    }
  };

  class type
  {
  private:
    value_t value;

  public:
    type () : value (control()) {}
    type (const expr::variant::type & v) : value (v) {}
    type (const structured_t & x) : value (x) {}

    expr::variant::type & operator [] (const signature::field_name_t & name)
    {
      structured_t map (boost::get<structured_t>(value));

      structured_t::iterator pos (map.find (name));

      if (pos == map.end())
        throw exception::unknown_field (name);

      return pos->second;
    }

    const expr::variant::type & get (const signature::field_name_t & name) const
    {
      structured_t map (boost::get<structured_t>(value));

      structured_t::const_iterator pos (map.find (name));

      if (pos == map.end())
        throw exception::unknown_field (name);

      return pos->second;
    }
      
    void bind (const signature::field_name_t & pref, context_t & c) const
    {
      boost::apply_visitor (visitor_bind (pref, c), value);
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

  inline bool operator == (const type & a, const type & b)
  {
    return a.value == b.value;
  }
  inline bool operator != (const type & a, const type & b)
  {
    return !(a == b);
  }

  std::ostream & operator << (std::ostream & s, const type & t)
  {
    static const visitor_show vs;

    return s << boost::apply_visitor (vs, t.value);
  }
}

#endif
