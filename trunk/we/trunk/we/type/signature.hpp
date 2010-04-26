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
    type () : desc_ (literal::CONTROL) {}
    template <typename T>
    type (const T & t) : desc_ (t) {}

    const desc_t & desc () const { return desc_; }

    friend std::ostream & operator << (std::ostream &, const type &);
  };

  namespace visitor
  {
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
}

#endif
