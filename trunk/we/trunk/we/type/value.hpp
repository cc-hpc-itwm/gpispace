// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_HPP
#define _WE_TYPE_VALUE_HPP

#include <we/type/literal.hpp>
#include <we/type/signature.hpp>
#include <we/type/error.hpp>

#include <we/util/show.hpp>

#include <string>
#include <map>

#include <boost/functional/hash.hpp>
#include <boost/variant.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

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
    // NOTE! sorted container neccessary for operator ==
    typedef std::map<signature::field_name_t, type> map_t;
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
    structured_t () : map () {}

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

  std::ostream & operator << (std::ostream &, const type &);

  namespace visitor
  {
    class is_structured : public boost::static_visitor<bool>
    {
    public:
      bool operator () (const structured_t &) const { return true; }
      bool operator () (const literal::type &) const { return false; }
    };

    class field : public boost::static_visitor<type &>
    {
    private:
      signature::field_name_t name;

    public:
      field (const signature::field_name_t & _name) : name (_name) {}

      type & operator () (structured_t & s) const
      {
        return s[name];
      }

      type & operator () (literal::type & l) const
      {
        throw std::runtime_error
          ("cannot get field " + name + " from the literal " + util::show(l));
      }
    };

    class mk_structured : public boost::static_visitor<type>
    {
    public:
      type operator () (const literal::type &) const
      {
        return structured_t();
      }

      type operator () (const structured_t & s) const
      {
        return s;
      }
    };

    class get_field : public boost::static_visitor<const type &>
    {
    private:
      signature::field_name_t name;

    public:
      get_field (const signature::field_name_t & _name) : name (_name) {}

      const type & operator () (const structured_t & s) const
      {
        structured_t::const_iterator pos (s.find (name));

        if (pos == s.end())
          throw std::runtime_error ("missing field " + name);

        return pos->second;
      }

      const type & operator () (const literal::type & l) const
      {
        throw std::runtime_error
          ("cannot get field " + name + " from the literal " + util::show(l));
      }
    };

    class has_field : public boost::static_visitor<bool>
    {
    private:
      signature::field_name_t name;

    public:
      has_field (const signature::field_name_t & _name) : name (_name) {}

      bool operator () (const structured_t & s) const
      {
        return s.has_field (name);
      }

      bool operator () (const literal::type &) const
      {
        return false;
      }
    };

    //this is *not* O(1), but safe
    class hash : public boost::static_visitor<std::size_t>
    {
    public:
      std::size_t operator () (const literal::type & v) const
      {
        return boost::hash_value(v);
      }

      std::size_t operator () (const structured_t & map) const
      {
        std::size_t v (0);

        for ( structured_t::map_t::const_iterator pos (map.begin())
            ; pos != map.end()
            ; ++pos
            )
          {
            std::size_t hash_field (0);

            boost::hash_combine (hash_field, pos->first);

            boost::hash_combine ( hash_field
                                , boost::apply_visitor (hash(), pos->second)
                                );

            v += hash_field;
          }

        return v;
      }
    };

    class show : public boost::static_visitor<std::ostream &>
    {
    private:
      std::ostream & s;

    public:
      show (std::ostream & _s) : s(_s) {}

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
            << field->first << " := " << field->second
            ;

        s << "]";

        return s;
      }
    };

    // binary visiting
    class require_type : public boost::static_visitor<type>
    {
    private:
      const signature::field_name_t & field_name;

    public:
      require_type (const signature::field_name_t & _field_name)
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
              throw ::type::error ("missing field " + sig->first);

            boost::apply_visitor
              ( require_type (field_name + "." + sig->first)
              , sig->second
              , pos->second
              );
          }

        for ( structured_t::const_iterator field (v.begin())
            ; field != v.end()
            ; ++field
            )
          if (!signature.has_field (field->first))
            throw ::type::error ("unknown field " + field->first);

        return v;
      }

      template<typename T, typename U>
      type operator () (const T & t, const U & u) const
      {
        throw ::type::error ("incompatible types: wanted type " + util::show (t) + " given value " + util::show (u));
      }
    };

    bool smaller_or_equal (const structured_t &, const structured_t &);

    class eq : public boost::static_visitor<bool>
    {
    public:
      bool operator () (const literal::type & x, const literal::type & y) const
      {
        return x == y;
      }

      bool operator () (const structured_t & x, const structured_t & y) const
      {
        //        return smaller_or_equal (x, y) && smaller_or_equal (y, x);

        structured_t::const_iterator pos_x (x.begin());
        structured_t::const_iterator pos_y (y.begin());
        const structured_t::const_iterator end_x (x.end());

        bool all_eq (  std::distance(pos_x, end_x)
                    == std::distance(pos_y, y.end())
                    );

        for ( ; all_eq && pos_x != end_x; ++pos_x, ++pos_y)
          all_eq =
            pos_x->first == pos_y->first
            &&
            boost::apply_visitor (eq(), pos_x->second, pos_y->second);

        return all_eq; // && (pos_x == end_x) && (pos_y == end_y);
      }

      template<typename A, typename B>
      bool operator () (const A &, const B &) const
      {
        return false;
      }
    };

    inline bool smaller_or_equal (const structured_t & x, const structured_t & y)
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
            : boost::apply_visitor (eq(), field->second, pos->second);
        }

      return all_eq;
    }

    class type_name : public boost::static_visitor<literal::type_name_t>
    {
    public:
      literal::type_name_t operator () (const literal::type & x) const
      {
        return boost::apply_visitor (literal::visitor::type_name(), x);
      }

      literal::type_name_t operator () (const structured_t & m) const
      {
        literal::type_name_t name ("[");

        for ( structured_t::const_iterator field (m.begin())
            ; field != m.end()
            ; ++field
            )
          name += ((field != m.begin()) ? ", " : "")
               +  field->first
               +  " :: "
               +  boost::apply_visitor (type_name(), field->second)
            ;

        name += "]";

        return name;
      }
    };

    template <typename T>
    class get_literal_value : public boost::static_visitor<T>
    {
    public:
      T const & operator () (const literal::type & literal) const
      {
        return boost::get<T> (literal);
      }

      T const &operator () (const value::structured_t & o) const
      {
        throw std::runtime_error ("bad_get: expected literal, got: " + util::show (o));
      }
    };
  }

  template <typename T, typename V>
  T const & get_literal_value (const V & v)
  {
    return boost::apply_visitor (visitor::get_literal_value<T const &>(), v);
  }

  inline const type &
  get_field (const signature::field_name_t & field, const type & v)
  {
    return boost::apply_visitor (visitor::get_field (field), v);
  }

  inline type &
  field (const signature::field_name_t & field, type & v)
  {
    return boost::apply_visitor (visitor::field (field), v);
  }

  inline std::ostream & operator << (std::ostream & s, const type & x)
  {
    return boost::apply_visitor (visitor::show (s), x);
  }
}

#endif
