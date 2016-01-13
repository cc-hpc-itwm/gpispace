#include <we/type/place.hpp>

namespace place
{
  const std::string& type::name() const
  {
    return _name;
  }

  const pnet::type::signature::signature_type& type::signature() const
  {
    return _signature;
  }

  const we::type::property::type& type::property() const
  {
    return _prop;
  }

  type::type ( const std::string& name
	     , const pnet::type::signature::signature_type& signature
             , boost::optional<bool> put_token
	     , const we::type::property::type& prop
	     )
    : _name (name)
    , _signature (signature)
    , _put_token (put_token)
    , _prop (prop)
  {}

  bool type::is_marked_for_put_token() const
  {
    return !!_put_token && _put_token.get();
  }
}
