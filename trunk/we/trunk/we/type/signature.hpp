// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_HPP
#define _WE_TYPE_SIGNATURE_HPP

#include <we/type/control.hpp>
#include <we/util/show.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <boost/serialization/variant.hpp>
#include <we/serialize/unordered_map.hpp>

#include <string>
#include <ostream>

namespace signature
{
  typedef std::string type_name_t;
  typedef std::string field_name_t;

  struct structured_t;

  typedef boost::variant< control
                        , type_name_t
                        , boost::recursive_wrapper<structured_t>
                        > desc_t;

  struct structured_t
  {
  public:
    typedef boost::unordered_map< field_name_t
                                , desc_t
                                > map_t;
    typedef map_t::const_iterator const_iterator;

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
    type () : desc_ (control()) {}
    template <typename T>
    type (const T & t) : desc_ (t) {}


    const desc_t & desc () const { return desc_; }

    friend std::ostream & operator << (std::ostream &, const type &);
  };

  class visitor_show : public boost::static_visitor<std::string>
  {
  public:
    std::string operator () (const control & x) const
    {
      return util::show (x);
    }

    std::string operator () (const type_name_t & t) const
    {
      return util::show (t);
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
          +  util::show (field->first)
          +  " := "
          +  util::show (field->second)
          ;

      s += "]";

      return s;
    }
  };

  inline std::ostream & operator << (std::ostream & os, const type & s)
  {
    static const visitor_show vs;

    return os << boost::apply_visitor (vs, s.desc());
  }
}

#endif
