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

  typedef boost::unordered_map<field_name_t,type_name_t> structured_t;
  typedef boost::variant<control, type_name_t, structured_t> desc_t;

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
