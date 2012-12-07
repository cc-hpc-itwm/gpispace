// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_PLACE_HPP
#define _WE_TYPE_PLACE_HPP

#include <we/type/literal.hpp>
#include <we/type/literal/name.hpp>
#include <we/type/signature.hpp>
#include <we/type/property.hpp>
#include <we/type/id.hpp>

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
    name_t name_;
    signature::type signature_;
    we::type::property::type prop_;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(name_);
      ar & BOOST_SERIALIZATION_NVP(signature_);
      ar & BOOST_SERIALIZATION_NVP(prop_);
    }

  public:
    const name_t &name() const { return name_; }
    void set_name(const name_t &name) { name_ = name; }

    const signature::type &signature() const { return signature_; }
    const we::type::property::type &property() const { return prop_; }
    we::type::property::type &property() { return prop_; }

    type ()
    {}

    explicit
    type ( const name_t & name
         , const literal::type_name_t & signature = literal::CONTROL()
         )
      : name_ (name)
      , signature_ (signature)
    {}

    template<typename T>
    type ( const name_t & name
         , const T & signature
         , const we::type::property::type & prop = we::type::property::type ()
         )
      : name_ (name), signature_ (signature), prop_ (prop)
    {}
  };

  inline std::ostream & operator << (std::ostream & s, const type & p)
  {
    return s << p.name();
  }

  inline bool operator == (const type & a, const type & b)
  {
    return a.name() == b.name();
  }

  inline std::size_t hash_value (const type & p)
  {
    boost::hash<type::name_t> h;

    return h(p.name());
  }

  template<typename NET>
  static const type::name_t &
  name (const NET & net, const petri_net::pid_t & pid)
  {
    return net.get_place(pid).name();
  }

  template<typename NET>
  static const signature::type &
  signature (const NET & net, const petri_net::pid_t & pid)
  {
    return net.get_place(pid).signature();
  }

  template<typename NET>
  static const we::type::property::type &
  property (const NET & net, const petri_net::pid_t & pid)
  {
    return net.get_place(pid).property();
  }
}

#endif
