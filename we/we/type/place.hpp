// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_PLACE_HPP
#define _WE_TYPE_PLACE_HPP

#include <we/type/literal.hpp>
#include <we/type/literal/name.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>

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
    we::type::property::type prop;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(name);
      ar & BOOST_SERIALIZATION_NVP(signature);
      ar & BOOST_SERIALIZATION_NVP(prop);
    }

  public:
    const name_t & get_name (void) const { return name; }
    const signature::type & get_signature (void) const { return signature; }
    const we::type::property::type & get_property (void) const { return prop; }
    we::type::property::type & property (void) { return prop; }

    type ()
    {}

    explicit
    type ( const name_t & _name
         , const literal::type_name_t & _type_name = literal::CONTROL()
         )
      : name (_name)
      , signature (_type_name)
    {}

    template<typename T>
    type ( const name_t & _name
         , const T & _signature
         , const we::type::property::type & _prop = we::type::property::type ()
         )
      : name (_name), signature (_signature), prop (_prop)
    {}

#ifdef BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
    type & operator= (const type &other)
    {
      if (this != &other)
      {
        name = other.name;
        signature = other.signature;
        prop = other.prop;
      }
      return *this;
    }
#endif // BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
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
  static const type::name_t &
  name (const NET & net, const petri_net::pid_t & pid)
  {
    return net.get_place(pid).get_name();
  }

  template<typename NET>
  static const signature::type &
  signature (const NET & net, const petri_net::pid_t & pid)
  {
    return net.get_place(pid).get_signature();
  }

  template<typename NET>
  static const we::type::property::type &
  property (const NET & net, const petri_net::pid_t & pid)
  {
    return net.get_place(pid).get_property();
  }
}

#endif
