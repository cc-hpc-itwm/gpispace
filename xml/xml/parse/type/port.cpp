// mirko.rahn@itwm.fraunhofer.de

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
      port_type::port_type ( const std::string & name
                           , const std::string & _type
                           , const fhg::util::maybe<std::string> & _place
                           , const id::port& id
                           , const id::function& parent
                           , id::mapper* id_mapper
                           )
        : _id (id)
        , _parent (parent)
        , _id_mapper (id_mapper)
        , _name (name)
        , type (_type)
        , place (_place)
        , prop ()
      {
        _id_mapper->put (_id, *this);
      }

      const id::port& port_type::id() const
      {
        return _id;
      }

      const std::string& port_type::name() const
      {
        return _name;
      }

      const id::function& port_type::parent() const
      {
        return _parent;
      }

      bool port_type::is_same (const port_type& other) const
      {
        return id() == other.id() && parent() == other.parent();
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


          void operator () (const net_type & net) const
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
              boost::optional<place_type> place
                (net.get_place (*port.place));

              if (!place)
              {
                throw error::port_connected_place_nonexistent
                  (direction, port.name(), *port.place, path);
              }

              if (place->type != port.type)
              {
                throw error::port_connected_type_error ( direction
                                                       , port
                                                       , *place
                                                       , path
                                                       );
              }

              if (direction == "tunnel")
              {
                if (not place->is_virtual())
                {
                  throw
                    error::tunnel_connected_non_virtual (port, *place, path);
                }

                if (port.name() != place->name())
                {
                  throw error::tunnel_name_mismatch (port, *place, path);
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

      void port_type_check ( const std::string & direction
                           , const port_type & port
                           , const boost::filesystem::path & path
                           , const state::type & state
                           , const function_type& fun
                           )
      {
        boost::apply_visitor
          (port_type_check_visitor (direction, port, path, state), fun.f);
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const port_type & p
                  , const std::string & direction
                  )
        {
          s.open (direction);
          s.attr ("name", p.name());
          s.attr ("type", p.type);
          s.attr ("place", p.place);

          ::we::type::property::dump::dump (s, p.prop);

          s.close();
        }
      }
    }
  }
}
