// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_PLACE_HPP
#define _WE_TYPE_PLACE_HPP

#include <we/type/id.hpp>
#include <we/type/control.hpp>
#include <we/type/signature.hpp>

#include <we/serialize/unordered_map.hpp>

#include <string>

#include <boost/serialization/nvp.hpp>

namespace place
{
  struct type
  {
  public:
    typedef std::string name_t;

  private:
    name_t name;
    signature::type signature;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(name);
      ar & BOOST_SERIALIZATION_NVP(signature);
    }

  public:
    const name_t & get_name (void) const { return name; }
    const signature::type & get_signature (void) const { return signature; }

    type (const name_t & _name)
      : name (_name)
      , signature (control())
    {}

    type ( const name_t & _name
         , const signature::type_name_t & _type_name
         )
      : name (_name)
      , signature (_type_name)
    {}

    type ( const name_t & _name
         , const signature::structured_t & _signature
         )
      : name (_name)
      , signature (_signature)
    {}
  };

  inline std::ostream & operator << (std::ostream & s, const type & p)
  {
    return s << p.get_name();
  }

  inline bool operator == (const type & a, const type & b)
  {
    return a.get_name() == b.get_name();
  }

  inline std::size_t hash_value (const type & p)
  {
    boost::hash<type::name_t> h;
    
    return h(p.get_name());
  }

  template<typename NET>
  static const type::name_t & name ( const NET & net
                                   , const petri_net::pid_t & pid
                                   )
  {
    return net.get_place(pid).get_name();
  }

  template<typename NET>
  static const signature::type & signature ( const NET & net
                                           , const petri_net::pid_t & pid)
  {
    return net.get_place(pid).get_signature();
  }
}

#endif
