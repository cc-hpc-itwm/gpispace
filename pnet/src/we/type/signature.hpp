
// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_HPP
#define _WE_TYPE_SIGNATURE_HPP

#include <we/type/literal/name.hpp>
#include <we/type/value/show.hpp>
#include <fhg/util/show.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/variant.hpp>
#include <we/serialize/unordered_map.hpp>

#include <string>
#include <ostream>

#include <map>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace signature
{
  struct structured_t;

  typedef boost::variant< std::string
                        , boost::recursive_wrapper<structured_t>
                        > desc_t;

  struct structured_t
  {
  public:
    // ordered map for equality!
    typedef std::map<std::string, desc_t> map_t;
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
    void insert (const std::string& n, const desc_t& d)
    {
      map.insert (std::make_pair (n,d));
    }
    desc_t& field (const std::string& n)
    {
      return map[n];
    }
    const desc_t & field (const std::string & field_name) const
    {
      const_iterator pos (map.find (field_name));

      if (pos == map.end())
        {
          throw std::runtime_error
            ("try to get non-existing field " + field_name);
        }

      return pos->second;
    }

    const_iterator begin (void) const { return map.begin(); }
    const_iterator end (void) const { return map.end(); }
    iterator begin (void) { return map.begin(); }
    iterator end (void) { return map.end(); }

    bool has_field (const std::string & field_name) const
    {
      return map.find (field_name) != map.end();
    }
  };

  namespace visitor
  {
    class dump : public boost::static_visitor<void>
    {
    private:
      const std::string & name;
      xml_util::xmlstream & s;

    public:
      dump (const std::string & _name, xml_util::xmlstream & _s)
        : name (_name)
        , s (_s)
      {}

      void operator () (const std::string & t) const
      {
        s.open ("field");
        s.attr ("name", name);
        s.attr ("type", t);
        s.close ();
      }

      void operator () (const structured_t & map) const
      {
        s.open ("struct");
        s.attr ("name", name);

        for ( structured_t::const_iterator field (map.begin())
            ; field != map.end()
            ; ++field
            )
          {
            boost::apply_visitor (dump (field->first, s), field->second);
          }

        s.close();
      }
    };

    class dump_token : public boost::static_visitor<void>
    {
    private:
      const std::string & name;
      xml_util::xmlstream & s;

      void open () const
      {
        if (name.size() > 0)
          {
            s.open ("field");
            s.attr ("name", name);
          }
        else
          {
            s.open ("token");
          }
      }

    public:
      dump_token (const std::string & _name, xml_util::xmlstream & _s)
        : name (_name)
        , s (_s)
      {}

      void operator () (const std::string & t) const
      {
        open();

        s.open ("value");
        s.content (t);
        s.close ();

        s.close();
      }

      void operator () (const structured_t & map) const
      {
        open();

        for ( structured_t::const_iterator field (map.begin())
            ; field != map.end()
            ; ++field
            )
          {
            boost::apply_visitor ( dump_token (field->first, s)
                                 , field->second
                                 );
          }

        s.close();
      }
    };

    class show : public boost::static_visitor<std::ostream &>
    {
    private:
      std::ostream & s;

    public:
      show (std::ostream & _s) : s(_s) {}

      std::ostream & operator () (const std::string & t) const
      {
        return s << fhg::util::show (t);
      }

      std::ostream & operator () (const structured_t & map) const
      {
        s << "[";

        for ( structured_t::const_iterator field (map.begin())
            ; field != map.end()
            ; ++field
            )
          s << ((field != map.begin()) ? ", " : "")
            << fhg::util::show (field->first)
            << " :: "
            << fhg::util::show (field->second)
            ;

        s << "]";

        return s;
      }
    };
  }

  class type
  {
  private:
    desc_t desc_;
    std::string nice_;

    std::string mk_nice (void)
    {
      std::ostringstream s;

      boost::apply_visitor (visitor::show (s), desc_);

      return s.str();
    }

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(desc_);
      ar & BOOST_SERIALIZATION_NVP(nice_);
    }
  public:
    type () : desc_(literal::CONTROL()), nice_ (mk_nice()) {}

    template <typename T>
    type (const T & t) : desc_ (t), nice_ (mk_nice()) {}

    template <typename T>
    type (const T & t, const std::string & s) : desc_ (t), nice_ (s) {}

    const desc_t & desc () const { return desc_; }
    const std::string & nice () const { return nice_; }

    friend std::ostream & operator << (std::ostream &, const type &);
    friend bool operator == (const type &, const type &);
    friend std::size_t hash_value (const type &);
  };

  namespace visitor
  {
    class has_field : public boost::static_visitor<bool>
    {
    private:
      const std::string _field;

    public:
      has_field (const std::string & field) : _field (field) {}

      bool operator () (structured_t & m) const { return m.has_field (_field); }
      bool operator () (std::string &) const { return false; }
    };

    class add_field : public boost::static_visitor<void>
    {
    private:
      const std::string field;
      const std::string type;
      const std::string msg;

    public:
      add_field ( const std::string & _field
                , const std::string & _type
                , const std::string & _msg = "signature"
                )
        : field (_field)
        , type (_type)
        , msg (_msg)
      {}

      void operator () (structured_t & map) const
      {
        map.insert (field, type);
      }

      void operator () (std::string &) const
      {
        throw std::runtime_error
          (msg + ": try to add a field to a non-structured " + msg);
      }
    };

    class get_field : public boost::static_visitor<desc_t &>
    {
    private:
      const std::string field;
      const std::string msg;

    public:
      get_field ( const std::string & _field
                , const std::string & _msg = "signature"
                )
        : field (_field)
        , msg (_msg)
      {}

      desc_t & operator () (structured_t & map) const
      {
        return map.field (field);
      }

      desc_t & operator () (std::string &) const
      {
        throw std::runtime_error
          (msg + ": try to get a field from a non-structured " + msg);
      }
    };

    class create_structured_field : public boost::static_visitor<void>
    {
    private:
      const std::string field;
      const std::string msg;

    public:
      create_structured_field ( const std::string & _field
                              , const std::string & _msg = "signature"
                              )
        : field (_field)
        , msg (_msg)
      {}

      void operator () (structured_t & map) const
      {
        if (map.has_field (field))
          {
            throw std::runtime_error (msg + ": try to create field "
                                     + field
                                     + " which is already there"
                                     );
          }

        map.insert (field, structured_t());
      }

      void operator () (std::string &) const
      {
        throw std::runtime_error
          (msg + ": try to create a field in a non-structured " + msg);
      }
    };
  }

  namespace
  {
    class get_or_create_structured_field_visitor
      : public boost::static_visitor<desc_t &>
    {
    private:
      const std::string field;
      const std::string msg;

    public:
      get_or_create_structured_field_visitor ( const std::string & _field
                                             , const std::string & _msg
                                             )
        : field (_field)
        , msg (_msg)
      {}

      desc_t & operator () (structured_t & map) const
      {
        if (!map.has_field (field))
        {
          map.insert (field, structured_t());
        }

        return map.field (field);
      }

      desc_t & operator () (std::string &) const
      {
        throw std::runtime_error
          (msg + ": try to create a field in a non-structured " + msg);
      }
    };

    template<typename T>
    class create_literal_field_visitor : public boost::static_visitor<void>
    {
    private:
      const std::string field;
      const T val;
      const std::string msg;

    public:
      create_literal_field_visitor ( const std::string & _field
                                   , const T & _val
                                   , const std::string & _msg
                                   )
        : field (_field)
        , val (_val)
        , msg (_msg)
      {}

      void operator () (structured_t & map) const
      {
        if (map.has_field (field))
          {
            throw std::runtime_error (msg + ": try to create field "
                                     + field
                                     + " which is already there"
                                     );
          }

        map.insert (field, val);
      }

      void operator () (std::string &) const
      {
        throw std::runtime_error
          (msg + ": try to create a field in a non-structured " + msg);
      }
    };
  }

  inline desc_t& get_or_create_structured_field ( desc_t& desc
                                                , const std::string& field
                                                , const std::string& msg
                                                = "signature"
                                                )
  {
    return boost::apply_visitor
      (get_or_create_structured_field_visitor (field, msg), desc);
  }

  template<typename T>
  inline void create_literal_field ( desc_t& desc
                                   , const std::string& field
                                   , const T& value
                                   , const std::string& msg
                                   = "signature"
                                   )
  {
    boost::apply_visitor
      (create_literal_field_visitor<T> (field, value, msg), desc);
  }


  inline std::ostream & operator << (std::ostream & os, const type & s)
  {
    return boost::apply_visitor (visitor::show (os), s.desc());
  }

  inline std::size_t hash_value (const type & s)
  {
    boost::hash<std::string> hasher;
    std::ostringstream os;
    os << s;
    return hasher (os.str());
  }

  inline bool operator == (const type & a, const type & b)
  {
    std::ostringstream oss_a, oss_b;
    oss_a << a; oss_b << b;
    return oss_a.str() == oss_b.str();
  }
}

#endif
