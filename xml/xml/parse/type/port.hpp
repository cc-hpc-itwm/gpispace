// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PORT_HPP
#define _XML_PARSE_TYPE_PORT_HPP

#include <string>
#include <iostream>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>

#include <we/type/property.hpp>

#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.hpp>

namespace xmlutil = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct port_type
      {
      public:
        std::string name;
        std::string type;
        boost::optional<std::string> place;
        we::type::property::type prop;

        port_type () : name (), type (), place (), prop () {}

        port_type ( const std::string & _name
                  , const std::string & _type
                  , const boost::optional<std::string> & _place
                  )
          : name (_name)
          , type (_type)
          , place (_place)
          , prop ()
        {}

        void specialize ( const type::type_map_type & map_in
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
      };

      // ******************************************************************* //

      template<typename NET>
      class port_type_check : public boost::static_visitor<void>
      {
      private:
        const std::string & direction;
        const port_type & port;
        const boost::filesystem::path & path;
        const state::type & state;

      public:
        port_type_check ( const std::string & _direction
                        , const port_type & _port
                        , const boost::filesystem::path & _path
                        , const state::type & _state
                        )
          : direction (_direction)
          , port (_port)
          , path (_path)
          , state (_state)
        {}

        void operator () (const NET & net) const
        {
          if (!port.place)
            {
              if (direction == "in")
                {
                  state.warn
                    (warning::port_not_connected (direction, port.name, path));
                }
              else
                {
                  throw error::port_not_connected (direction, port.name, path);
                }
            }
          else
            {
              place_type place;

              if (!net.get_place (*port.place, place))
                {
                  throw error::port_connected_place_nonexistent
                    (direction, port.name, *port.place, path);
                }

              if (place.type != port.type)
                {
                  throw
                    error::port_connected_type_error<port_type,place_type>
                    (direction, port, place, path);
                }
            }
        }

        template<typename T>
        void operator () (const T &) const
        {
          if (port.place)
            {
              throw error::port_connected_place_nonexistent
                (direction, port.name, *port.place, path);
            }
        }
      };

      // ******************************************************************* //

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const port_type & p
                         , const std::string & direction
                         )
        {
          s.open (direction);
          s.attr ("name", p.name);
          s.attr ("type", p.type);
          s.attr ("place", p.place);

          ::we::type::property::dump::dump (s, p.prop);

          s.close();
        }
      } // namespace dump
    }
  }
}

#endif
