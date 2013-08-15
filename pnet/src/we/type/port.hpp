// {alexander.petry,bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef WE_TYPE_PORT_HPP
#define WE_TYPE_PORT_HPP 1

#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>
#include <we2/type/signature.hpp>
#include <we2/type/signature/serialize.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/nvp.hpp>

#include <iosfwd>
#include <limits>
#include <string>

namespace we
{
  namespace type
  {
    enum PortDirection
      { PORT_IN
      , PORT_OUT
      , PORT_TUNNEL
      };

    std::string enum_to_string (const PortDirection&);

    std::ostream& operator<< (std::ostream&, const PortDirection&);

    struct port_t
    {
    public:
      port_t ()
        : _name("default")
        , _direction(PORT_IN)
        , _associated_place(petri_net::place_id_invalid())
      {}

      port_t ( const std::string & name
             , PortDirection direction
             , const pnet::type::signature::signature_type& signature
             , const we::type::property::type prop
             = we::type::property::type()
             )
        : _name(name)
        , _direction(direction)
        , _signature(signature)
        , _associated_place(petri_net::place_id_invalid())
        , _properties(prop)
      {}

      port_t ( const std::string & name
             , PortDirection direction
             , const pnet::type::signature::signature_type& signature
             , const petri_net::place_id_type& place_id
             , const we::type::property::type prop
             = we::type::property::type()
             )
        : _name(name)
        , _direction(direction)
        , _signature(signature)
        , _associated_place(place_id)
        , _properties(prop)
      {}

      std::string & name() { return _name; }
      const std::string & name() const { return _name; }

      PortDirection direction() const { return _direction; }
      const pnet::type::signature::signature_type& signature() const { return _signature; }
      const petri_net::place_id_type& associated_place() const { return _associated_place; }
      petri_net::place_id_type& associated_place() { return _associated_place; }
      const we::type::property::type & property() const { return _properties; }

      bool is_input() const { return _direction == PORT_IN; }
      bool is_output() const { return _direction == PORT_OUT; }
      bool is_tunnel() const { return _direction == PORT_TUNNEL; }
      bool has_associated_place() const { return _associated_place != petri_net::place_id_invalid(); }

    private:
      std::string _name;
      PortDirection _direction;
      pnet::type::signature::signature_type _signature;
      petri_net::place_id_type _associated_place;
      we::type::property::type _properties;

      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(_name);
        ar & BOOST_SERIALIZATION_NVP(_direction);
        ar & BOOST_SERIALIZATION_NVP(_signature);
        ar & BOOST_SERIALIZATION_NVP(_associated_place);
        ar & BOOST_SERIALIZATION_NVP(_properties);
      }
    };

    std::ostream& operator<< (std::ostream&, const port_t&);
  }
}

#endif
