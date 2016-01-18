// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <xml/parse/type/port.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/error.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>

#include <we/type/signature/is_literal.hpp>

#include <fhg/assert.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      port_type::port_type ( const util::position_type& position_of_definition
                           , const std::string & name
                           , const std::string & type
                           , const boost::optional<std::string> & _place
                           , const we::type::PortDirection& direction
                           , const we::type::property::type& properties
                           , boost::optional<pnet::type::signature::signature_type> signature
                           )
        : with_position_of_definition (position_of_definition)
        , _name (name)
        , _type (type)
        , _signature (std::move (signature))
        , place (_place)
        , _direction (direction)
        , _properties (properties)
      {}

      const std::string& port_type::name() const
      {
        return _name;
      }

      const std::string& port_type::type() const
      {
        return _type;
      }

      pnet::type::signature::signature_type port_type::signature() const
      {
        //! \note assume post processing pass (resolve_types_recursive)
        return _signature.get();
      }
      void port_type::resolve_types_recursive
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        if (pnet::type::signature::is_literal (_type))
        {
          _signature = pnet::type::signature::signature_type (_type);
        }
        else
        {
          auto it (known.find (_type));
          if (it == known.end())
          {
            throw error::port_with_unknown_type
              (direction(), name(), type(), position_of_definition().path());
          }
          _signature = it->second;
        }
      }

      port_type port_type::specialized ( const type::type_map_type & map_in
                                       , const state::type &
                                       ) const
      {
        const type::type_map_type::const_iterator
          mapped (map_in.find (type()));

        return port_type ( position_of_definition()
                         , name()
                         , mapped != map_in.end() ? mapped->second : type()
                         , place
                         , direction()
                         , properties()
                         , _signature
                         );
      }

      namespace
      {
        class type_checker : public boost::static_visitor<void>
        {
        private:
          const port_type& _port;
          const boost::filesystem::path& _path;
          const state::type& _state;

        public:
          type_checker ( const port_type& port
                       , const boost::filesystem::path& path
                       , const state::type& state
                       )
            : _port (port)
            , _path (path)
            , _state (state)
          { }

          void operator() (id::ref::net const& net) const
          {
            if (not _port.place)
            {
              if (_port.direction() == we::type::PORT_IN)
              {
                _state.warn (warning::port_not_connected (_port, _path));
              }
              else
              {
                throw error::port_not_connected (_port, _path);
              }
            }
            else
            {
              boost::optional<place_type const&>
                place (_port.resolved_place (net));

              if (not place)
              {
                throw error::port_connected_place_nonexistent (_port, _path);
              }

              if (place->signature() != _port.signature())
              {
                throw error::port_connected_type_error (_port, *place, _path);
              }

              if (_port.direction() == we::type::PORT_TUNNEL)
              {
                if (not place->is_virtual())
                {
                  throw
                    error::tunnel_connected_non_virtual (_port, *place, _path);
                }

                if (_port.name() != place->name())
                {
                  throw error::tunnel_name_mismatch (_port, *place, _path);
                }
              }
            }
          }

          void operator() (const expression_type&) const
          {
            if (_port.place)
            {
              throw error::port_connected_place_nonexistent (_port, _path);
            }
          }

          void operator() (const module_type&) const
          {
            if (_port.place)
            {
              throw error::port_connected_place_nonexistent (_port, _path);
            }
          }
        };
      }

      void port_type::type_check ( const boost::filesystem::path& path
                                 , const state::type& state
                                 , function_type const& parent
                                 ) const
      {
        boost::apply_visitor
          (type_checker (*this, path, state), parent.content());
      }

      const we::type::PortDirection& port_type::direction() const
      {
        return _direction;
      }

      boost::optional<place_type const&> port_type::resolved_place
        (id::ref::net const& parent) const
      {
        if (!place)
        {
          return boost::none;
        }

        return parent.get().places().get (*place);
      }

      const we::type::property::type& port_type::properties() const
      {
        return _properties;
      }

      port_type::unique_key_type port_type::unique_key() const
      {
        return std::make_pair (name(), direction());
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const port_type& p)
        {
          s.open (we::type::enum_to_string (p.direction()));
          s.attr ("name", p.name());
          s.attr ("type", p.type());
          s.attr ("place", p.place);

          ::we::type::property::dump::dump (s, p.properties());

          s.close();
        }
      }
    }
  }
}
