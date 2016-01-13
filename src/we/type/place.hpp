#pragma once

#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>

namespace place
{
  //! \todo add properties
  struct type
  {
  public:
    const std::string& name() const;

    const pnet::type::signature::signature_type& signature() const;
    const we::type::property::type& property() const;

    //! \todo eliminate the need for the default constructor
    type () = default;
    type ( const std::string& name
         , const pnet::type::signature::signature_type& signature
         , boost::optional<bool> put_token
         , const we::type::property::type& prop = we::type::property::type ()
	 );

    bool is_marked_for_put_token() const;

  private:
    //! \todo maybe one should factor out the (name, sig, prop)-pattern
    // into a base class
    std::string _name;
    pnet::type::signature::signature_type _signature;
    boost::optional<bool> _put_token;
    we::type::property::type _prop;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_name);
      ar & BOOST_SERIALIZATION_NVP(_signature);
      ar & BOOST_SERIALIZATION_NVP(_put_token);
      ar & BOOST_SERIALIZATION_NVP(_prop);
    }
  };
}
