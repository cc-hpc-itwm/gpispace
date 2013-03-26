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

  const signature::type& type::signature() const
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
    : _name ("<<place>>")
    , _signature ("<<signature>>")
  {}

  type::type ( const std::string& name
	     , const signature::type& signature
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

  bool operator== (const type& a, const type& b)
  {
    return a.name() == b.name();
  }

  std::size_t hash_value (const type& p)
  {
    boost::hash<std::string> h;

    return h(p.name());
  }
}
