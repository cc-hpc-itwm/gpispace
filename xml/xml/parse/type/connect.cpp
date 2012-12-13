// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/net.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      connect_type::connect_type ( ID_CONS_PARAM(connect)
                                 , PARENT_CONS_PARAM(transition)
                                 , const std::string& place
                                 , const std::string& port
                                 , const ::petri_net::edge::type& direction
                                 , const we::type::property::type& properties
                                 )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _place (place)
        , _port (port)
        , _direction (direction)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& connect_type::place() const
      {
        return _place;
      }
      const std::string& connect_type::port() const
      {
        return _port;
      }

      boost::optional<const id::ref::place&> connect_type::resolved_place() const
      {
        return parent()->parent()->places().get (place());
      }
      boost::optional<const id::ref::port&> connect_type::resolved_port() const
      {
        return parent()->resolved_function().get().ports().get
          ( std::make_pair ( port()
                           , petri_net::edge::is_PT (direction())
                           ? we::type::PORT_IN
                           : we::type::PORT_OUT
                           )
          );
      }

      const ::petri_net::edge::type& connect_type::direction() const
      {
        return _direction;
      }
      const ::petri_net::edge::type& connect_type::direction
        (const ::petri_net::edge::type& direction_)
      {
        return _direction = direction_;
      }

      const std::string& connect_type::place (const std::string& place)
      {
        return _place = place;
      }

      const we::type::property::type& connect_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& connect_type::properties()
      {
        return _properties;
      }

      connect_type::unique_key_type connect_type::unique_key() const
      {
        return boost::make_tuple
          (_place, _port, petri_net::edge::is_PT (_direction));
      }


      id::ref::connect connect_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return connect_type
          ( new_id
          , new_mapper
          , parent
          , _place
          , _port
          , _direction
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const connect_type& c)
        {
          s.open ("connect-" + petri_net::edge::enum_to_string (c.direction()));
          s.attr ("port", c.port());
          s.attr ("place", c.place());

          ::we::type::property::dump::dump (s, c.properties());

          s.close ();
        }
      }
    }
  }
}
