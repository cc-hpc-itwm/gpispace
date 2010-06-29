// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_HPP
#define _WE_TYPE_SIGNATURE_HPP

#include <we/type/literal/name.hpp>
#include <we/util/show.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <boost/serialization/variant.hpp>
#include <we/serialize/unordered_map.hpp>

#include <string>
#include <ostream>

namespace signature
{
  typedef std::string field_name_t;

  struct structured_t;

  typedef boost::variant< literal::type_name_t
                        , boost::recursive_wrapper<structured_t>
                        > desc_t;

  typedef boost::unordered_map<field_name_t, desc_t> set_type;

  struct structured_t
  {
  public:
    typedef boost::unordered_map< field_name_t
                                , desc_t
                                > map_t;
    typedef map_t::const_iterator const_iterator;
    typedef map_t::iterator iterator;

  private:
    map_t map;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(map);
    }
  public:
    desc_t & operator [] (const field_name_t & field_name)
    {
      return map[field_name];
    }

    const_iterator begin (void) const { return map.begin(); }
    const_iterator end (void) const { return map.end(); }
    iterator begin (void) { return map.begin(); }
    iterator end (void) { return map.end(); }

    bool has_field (const field_name_t & field_name) const
    {
      return map.find (field_name) != map.end();
    }
  };

  class type
  {
  private:
    desc_t desc_;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(desc_);
    }
  public:
    type () : desc_ (literal::CONTROL) {}
    template <typename T>
    type (const T & t) : desc_ (t) {}

    const desc_t & desc () const { return desc_; }

    friend std::ostream & operator << (std::ostream &, const type &);
    friend bool operator == (const type &, const type &);
    friend std::size_t hash_value (const type &);
  };

  namespace visitor
  {
    class add_field : public boost::static_visitor<void>
    {
    private:
      const field_name_t field;
      const literal::type_name_t type;

    public:
      add_field ( const field_name_t & _field
                , const literal::type_name_t & _type
                )
        : field (_field)
        , type (_type)
      {}

      void operator () (structured_t & map) const
      {
        map[field] = type;
      }
      
      void operator () (literal::type_name_t &) const
      {
        throw std::runtime_error
          ("signature: try to add a field to a non-structured signature");
      }
    };

    class get_field : public boost::static_visitor<desc_t &>
    {
    private:
      const field_name_t field;

    public:
      get_field (const field_name_t & _field) : field (_field) {}

      desc_t & operator () (structured_t & map) const
      {
        return map[field];
      }
      
      desc_t & operator () (literal::type_name_t &) const
      {
        throw std::runtime_error
          ("signature: try to get a field from a non-structured signature");
      }
    };

    class create_structured_field : public boost::static_visitor<void>
    {
    private:
      const field_name_t field;

    public:
      create_structured_field (const std::string & _field) : field (_field) {}

      void operator () (structured_t & map) const
      {
        if (map.has_field (field))
          {
            throw std::runtime_error ("signature: try to create field " 
                                     + field 
                                     + " which is already there"
                                     );
          }

        map[field] = structured_t();
      }

      void operator () (literal::type_name_t &) const
      {
        throw std::runtime_error
          ("signature: try to create a field in a non-strutured signature");
      }
    };

    class get_literal_type_name 
      : public boost::static_visitor<literal::type_name_t>
    {
    public:
      literal::type_name_t operator () (const literal::type_name_t & t) const
      {
        return t;
      }
      literal::type_name_t operator () (const structured_t &) const
      {
        throw std::runtime_error
          ("signature: try to get a literal typename from a structured type");
      }
    };

    class resolve : public boost::static_visitor<bool>
    {
    private:
      set_type & sig_set;
      literal::name name;
      
    public:
      resolve (set_type & _sig_set) : sig_set (_sig_set), name() {}
      
      bool operator () (literal::type_name_t & t) const
      {
        return name.valid (t);
      }

      bool operator () (structured_t & map) const
      {
        for ( structured_t::map_t::iterator pos (map.begin())
            ; pos != map.end()
            ; ++pos
            )
          {
            const bool resolved (boost::apply_visitor (*this, pos->second));
            
            if (!resolved)
              {
                const literal::type_name_t name
                  (boost::apply_visitor (get_literal_type_name(), pos->second));

                set_type::const_iterator res (sig_set.find (name));

                if (res == sig_set.end())
                  {
                    throw std::runtime_error ("signature: cannot resolve "
                                             + pos->first + " :: " + name
                                             );
                  }

                pos->second = res->second;

                boost::apply_visitor (*this, pos->second);
              }
          }

        return true;
      }
    };

    class show : public boost::static_visitor<std::ostream &>
    {
    private:
      std::ostream & s;

    public:
      show (std::ostream & _s) : s(_s) {};

      std::ostream & operator () (const literal::type_name_t & t) const
      {
        return s << util::show (t);
      }

      std::ostream & operator () (const structured_t & map) const
      {
        s << "[";

        for ( structured_t::const_iterator field (map.begin())
            ; field != map.end()
            ; ++field
            )
          s << ((field != map.begin()) ? ", " : "")
            << util::show (field->first)
            << " :: "
            << util::show (field->second)
            ;

        s << "]";

        return s;
      }
    };
  }

  inline std::ostream & operator << (std::ostream & os, const type & s)
  {
    return boost::apply_visitor (visitor::show (os), s.desc());
  }

  inline std::size_t hash_value (const type & s)
  {
    std::ostringstream oss;
    oss << s;
    boost::hash<std::string> hasher;
    return hasher (oss.str());
  }

  inline bool operator == (const type & a, const type & b)
  {
    std::ostringstream oss_a, oss_b;
    oss_a << a; oss_b << b;
    return oss_a.str() == oss_b.str();
  }
}

#endif
