// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_HPP
#define _WE_TYPE_VALUE_HPP

#include <we/type/literal.hpp>
#include <we/type/signature.hpp>

#include <we/util/show.hpp>

#include <string>

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>

#include <iostream>

namespace value
{
  struct structured_t;

  typedef boost::variant< literal::type
                        , boost::recursive_wrapper<structured_t>
                        > type;

  struct structured_t
  {
  public:
    typedef boost::unordered_map< signature::field_name_t
                                , type
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
    type & operator [] (const signature::field_name_t & field_name)
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

  class visitor_hash : public boost::static_visitor<std::size_t>
  {
  public:
    std::size_t operator () (const literal::type & v) const
    {
      return boost::hash_value(v);
    }

    std::size_t operator () (const structured_t & map) const
    {
      const structured_t::map_t::const_iterator pos (map.begin());

      return (pos == map.end()) 
        ? 3141 
        : boost::apply_visitor (visitor_hash(), pos->second);
    }
  };

  class visitor_show : public boost::static_visitor<std::ostream &>
  {
  private:
    std::ostream & s;
  public:
    visitor_show (std::ostream & _s) : s(_s) {}

    std::ostream & operator () (const literal::type & v) const
    {
      return s << literal::show (v);
    }

    std::ostream & operator () (const structured_t & map) const
    {
      s << "[";

      for ( structured_t::const_iterator field (map.begin())
          ; field != map.end()
          ; ++field
          )
        s << ((field != map.begin()) ? ", " : "")
          << field->first 
          << " := "
          << boost::apply_visitor (visitor_show(s), field->second)
          ;

      s << "]";

      return s;
    }
  };

  // binary visiting
  class visitor_require_type : public boost::static_visitor<type>
  {
  private:
    const signature::field_name_t & field_name;

  public:
    visitor_require_type (const signature::field_name_t & _field_name)
      : field_name (_field_name)
    {}

    type operator () ( const literal::type_name_t & type_name
                     , const literal::type & v
                     ) const
    {
      return literal::require_type (field_name, type_name, v);
    }

    type operator () ( const signature::structured_t & signature
                     , const structured_t & v
                     ) const
    {
      for ( signature::structured_t::const_iterator sig (signature.begin())
          ; sig != signature.end()
          ; ++sig
          )
        {
          const structured_t::const_iterator pos (v.find (sig->first));

          if (!v.has_field (sig->first))
            throw literal::exception::type_error ( "missing field " 
                                                 + sig->first
                                                 );

          boost::apply_visitor
            ( visitor_require_type (field_name + "." + sig->first)
            , sig->second
            , pos->second
            );
        }

      for ( structured_t::const_iterator field (v.begin())
          ; field != v.end()
          ; ++field
          )
        if (!signature.has_field (field->first))
          throw literal::exception::type_error ( "unknown field " 
                                               + field->first
                                               );

      return v;
    }

    template<typename T, typename U>
    type operator () (const T &, const U &) const
    {
      throw literal::exception::type_error ("incompatible types");
    }
  };

  bool smaller_or_equal (const structured_t &, const structured_t &);

  class visitor_eq : public boost::static_visitor<bool>
  {
  public:
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
}

#endif
