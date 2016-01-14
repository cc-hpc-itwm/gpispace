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
                                 , const util::position_type& pod
                                 , const std::string& place
                                 , const std::string& port
                                 , const ::we::edge::type& direction
                                 , const we::type::property::type& properties
                                 )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
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

      const ::we::edge::type& connect_type::direction() const
      {
        return _direction;
      }

      const std::string& connect_type::place_impl (const std::string& place)
      {
        return _place = place;
      }
      const std::string& connect_type::place (const std::string& place)
      {
        if (has_parent())
        {
          parent()->connection_place (make_reference_id(), place);
          return _place;
        }

        return place_impl (place);
      }

      const we::type::property::type& connect_type::properties() const
      {
        return _properties;
      }

      connect_type::unique_key_type connect_type::unique_key() const
      {
        return std::make_tuple
          (_place, _port, we::edge::is_PT (_direction));
      }


      id::ref::connect connect_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        , boost::optional<we::edge::type> direction
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return connect_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _place
          , _port
          , direction.get_value_or (_direction)
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const connect_type& c)
        {
          s.open ("connect-" + we::edge::enum_to_string (c.direction()));
          s.attr ("port", c.port());
          s.attr ("place", c.place());

          ::we::type::property::dump::dump (s, c.properties());

          s.close ();
        }
      }
    }
  }
}
