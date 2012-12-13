// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <xml/parse/type/port.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      port_type::port_type ( ID_CONS_PARAM(port)
                           , PARENT_CONS_PARAM(function)
                           , const std::string & name
                           , const std::string & _type
                           , const boost::optional<std::string> & _place
                           , const we::type::PortDirection& direction
                           , const we::type::property::type& properties
                           )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , type (_type)
        , place (_place)
        , _direction (direction)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& port_type::name() const
      {
        return _name;
      }

      void port_type::specialize ( const type::type_map_type & map_in
                                 , const state::type &
                                 )
      {
        const type::type_map_type::const_iterator
          mapped (map_in.find (type));

        if (mapped != map_in.end())
        {
          type = mapped->second;
        }
      }

      namespace
      {
        class port_type_check_visitor : public boost::static_visitor<void>
        {
        private:
          const std::string & direction;
          const port_type & port;
          const boost::filesystem::path & path;
          const state::type & state;

        public:
          port_type_check_visitor ( const std::string & _direction
                                  , const port_type & _port
                                  , const boost::filesystem::path & _path
                                  , const state::type & _state
                                  )
            : direction (_direction)
            , port (_port)
            , path (_path)
            , state (_state)
          { }


          void operator () (const id::ref::net & id_net) const
          {
            if (not port.place)
            {
              if (direction == "in")
              {
                state.warn
                  (warning::port_not_connected (direction, port.name(), path));
              }
              else
              {
                throw error::port_not_connected (direction, port.name(), path);
              }
            }
            else
            {
              boost::optional<const id::ref::place&>
                id_place (id_net.get().places().get (*port.place));

              if (not id_place)
              {
                throw error::port_connected_place_nonexistent
                  (direction, port.name(), *port.place, path);
              }

              const place_type& place (id_place->get());

              if (place.type != port.type)
              {
                throw error::port_connected_type_error ( direction
                                                       , port
                                                       , place
                                                       , path
                                                       );
              }

              if (direction == "tunnel")
              {
                if (not place.is_virtual())
                {
                  throw
                    error::tunnel_connected_non_virtual (port, place, path);
                }

                if (port.name() != place.name())
                {
                  throw error::tunnel_name_mismatch (port, place, path);
                }
              }
            }
          }

          template<typename T>
          void operator () (const T &) const
          {
            if (port.place)
            {
              throw error::port_connected_place_nonexistent
                (direction, port.name(), *port.place, path);
            }
          }
        };
      }

      void port_type::type_check ( const std::string & direction
                                 , const boost::filesystem::path & path
                                 , const state::type & state
                                 ) const
      {
        assert (has_parent());

        boost::apply_visitor
          ( port_type_check_visitor (direction, *this, path, state)
          , parent()->f
          );
      }

      const we::type::PortDirection& port_type::direction() const
      {
        return _direction;
      }
      const we::type::PortDirection& port_type::direction
        (const we::type::PortDirection& direction_)
      {
        return _direction = direction_;
      }

      const we::type::property::type& port_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& port_type::properties()
      {
        return _properties;
      }

      port_type::unique_key_type port_type::unique_key() const
      {
        return std::make_pair (name(), direction());
      }

      id::ref::port port_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return port_type
          ( new_id
          , new_mapper
          , parent
          , _name
          , type
          , place
          , _direction
          , _properties
          ).make_reference_id();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const port_type& p)
        {
          s.open (we::type::enum_to_string (direction));
          s.attr ("name", p.name());
          s.attr ("type", p.type);
          s.attr ("place", p.place);

          ::we::type::property::dump::dump (s, p.properties());

          s.close();
        }
      }
    }
  }
}
