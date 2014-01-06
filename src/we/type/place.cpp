// mirko.rahn@itwm.fraunhofer.de

#include <we/type/place.hpp>

#include <boost/functional/hash.hpp>

#include <ostream>

namespace place
{
  const std::string& type::name() const
  {
    return _name;
  }
  const std::string& type::name (const std::string& name_)
  {
    return _name = name_;
  }

  const pnet::type::signature::signature_type& type::signature() const
  {
    return _signature;
  }

  const we::type::property::type& type::property() const
  {
    return _prop;
  }
  we::type::property::type& type::property()
  {
    return _prop;
  }

  type::type ()
    : _name()
    , _signature()
  {}

  type::type ( const std::string& name
	     , const pnet::type::signature::signature_type& signature
	     , const we::type::property::type& prop
	     )
    : _name (name)
    , _signature (signature)
    , _prop (prop)
  {}

  std::ostream& operator<< (std::ostream& s, const type& p)
  {
    return s << p.name();
  }
}
