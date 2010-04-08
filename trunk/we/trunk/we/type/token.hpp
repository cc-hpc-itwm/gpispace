// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_TOKEN_HPP
#define _WE_TYPE_TOKEN_HPP

#include <we/expr/variant/variant.hpp>
#include <we/expr/eval/context.hpp>

#include <string>
#include <stdexcept>

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <iostream>

namespace we
{
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

    class type
    {
    public:
      typedef std::string field_name_t;
      typedef boost::unordered_map<field_name_t,expr::variant::type> map_t;

    private:
      map_t map;
      std::size_t arity;

    public:
      type() : map (), arity (0) {}
      type (const expr::variant::type & v) : map (), arity (1) { map[""] = v; }
      type (const map_t & m) : map (m), arity (m.size()) {}

      expr::variant::type & operator [] (const field_name_t & name)
      {
        map_t::iterator pos (map.find (name));

        if (pos == map.end())
          throw exception::unknown_field (name);

        return pos->second;
      }

      const expr::variant::type & get (const field_name_t & name) const
      {
        map_t::const_iterator pos (map.find (name));

        if (pos == map.end())
          throw exception::unknown_field (name);

        return pos->second;
      }
      
      typedef expr::eval::context<std::string> context_t;

      void bind (const std::string & pref, context_t & c) const
      {
        switch (arity)
          {
          case 0: c.bind (pref, expr::variant::control()); break;
          case 1: c.bind (pref, get ("")); break;
          default:
            for (map_t::const_iterator f (map.begin()); f != map.end(); ++f)
              c.bind (pref + "." + f->first, f->second);
            break;
          }
      }

      friend std::ostream & operator << (std::ostream &, const type &);
      friend bool operator == (const type &, const type &);
      friend bool operator != (const type &, const type &);
      friend std::size_t hash_value (const type &);
    };

    // WORK HERE! implement a better hash function!?
    inline std::size_t hash_value (const we::token::type & t)
    {
      if (t.arity == 0)
        return 0;
      else
        {
          boost::hash<we::token::type::map_t::value_type> h;

          return h (*(t.map.begin()));
        }
    }

    inline bool operator == (const type & a, const type & b)
    {
      return a.arity == b.arity && a.map == b.map;
    }
    inline bool operator != (const type & a, const type & b)
    {
      return !(a == b);
    }

    std::ostream & operator << (std::ostream & s, const type & t)
    {
      switch (t.arity)
        {
        case 0: return s << expr::variant::show (expr::variant::control());
        case 1: return s << expr::variant::show (t.map.begin()->second);
        default:
          s << "[";

          for ( type::map_t::const_iterator field (t.map.begin())
              ; field != t.map.end()
              ; ++field
              )
            s << ((field != t.map.begin()) ? ", " : "")
              << field->first 
              << " := "
              << expr::variant::show (field->second)
              ;
          
          return s << "]";
        }
    }
  }
}

#endif
