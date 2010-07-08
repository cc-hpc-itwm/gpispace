// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PORT_HPP
#define _XML_PARSE_TYPE_PORT_HPP

#include <string>
#include <iostream>

#include <parse/util/maybe.hpp>
#include <parse/error.hpp>
#include <parse/state.hpp>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>

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
        maybe<std::string> place;

        port_type () : name (), type (), place () {}

        port_type ( const std::string & _name
                  , const std::string & _type
                  , const maybe<std::string> & _place
                  )
          : name (_name)
          , type (_type)
          , place (_place)
        {}
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
          if (port.place.isNothing())
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

              const bool found (net.get_place (*port.place, place));

              if (!found)
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
          if (port.place.isJust())
            {
              throw error::port_connected_place_nonexistent
                (direction, port.name, *port.place, path);
            }
        }
      };

      // ******************************************************************* //

      std::ostream & operator << (std::ostream & s, const port_type & p)
      {
        return s << "port ("
                 << "name = " << p.name
                 << ", type = " << p.type
                 << ", place = " << p.place
                 << ")"
          ;
      }
    }
  }
}

#endif
