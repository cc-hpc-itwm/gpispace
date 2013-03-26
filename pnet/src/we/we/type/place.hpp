// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_PLACE_HPP
#define _WE_TYPE_PLACE_HPP

#include <we/type/signature.hpp>
#include <we/type/property.hpp>

#include <boost/serialization/nvp.hpp>

#include <string>
#include <iosfwd>

namespace place
{
  struct type
  {
  public:
    const std::string& name() const;
    const std::string& name (const std::string&);

    const signature::type& signature() const;
    const we::type::property::type& property() const;
    we::type::property::type& property();

    //! \todo eliminate the need for the default constructor
    type ();
    type ( const std::string& name
         , const signature::type& signature
         , const we::type::property::type& prop = we::type::property::type ()
	 );

  private:
    //! \todo maybe one should factor out the (name, sig, prop)-pattern 
    // into a base class
    std::string _name;
    signature::type _signature;
    we::type::property::type _prop;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_name);
      ar & BOOST_SERIALIZATION_NVP(_signature);
      ar & BOOST_SERIALIZATION_NVP(_prop);
    }
  };

  std::ostream& operator<< (std::ostream&, const type&);
  bool operator== (const type&, const type&);
  std::size_t hash_value (const type&);
}

#endif
