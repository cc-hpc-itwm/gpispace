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

#include <boost/unordered_map.hpp>

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
    typedef std::map<signature::field_name_t, type> map_t;
    // typedef boost::unordered_map<signature::field_name_t, type> map_t;
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
        throw std::runtime_error ("cannot get field " + name + " from the literal " + util::show(l));
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
      type operator () (const T &, const U &) const
      {
        throw ::type::error ("incompatible types");
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
               +  ": "
               +  boost::apply_visitor (type_name(), field->second)
            ;

        name += "]";

        return name;
      }
    };
  }

  inline const type & require_type ( const signature::field_name_t & field
                                   , const literal::type_name_t & req
                                   , const type & x
                                   )
  {
    const literal::type_name_t has ( boost::apply_visitor ( visitor::type_name()
                                                          , x
                                                          )
                                   );

    if (has != req)
      throw ::type::error (field, req, has);

    return x;
  }

  std::ostream & operator << (std::ostream & s, const type & x)
  {
    return boost::apply_visitor (visitor::show (s), x);
  }
}

#endif
