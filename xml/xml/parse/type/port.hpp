// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PORT_HPP
#define _XML_PARSE_TYPE_PORT_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/id_type.hpp>

#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>

#include <boost/filesystem.hpp>
#include <boost/variant.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct port_type
      {
      public:
        port_type ( const std::string & _name
                  , const std::string & _type
                  , const fhg::util::maybe<std::string> & _place
                  , const id::port& id
                  , const id::function& parent
                  );

        const id::port& id() const;
        const id::transition& parent() const;

        bool is_same (const port_type& other) const;

        void specialize ( const type::type_map_type & map_in
                        , const state::type &
                        );
      private:
        id::port _id;
        id::function _parent;

      public:
        std::string name;
        std::string type;
        fhg::util::maybe<std::string> place;
        we::type::property::type prop;
      };

      // ******************************************************************* //

      //! \todo This should be in the cpp file only.
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
                        );

        void operator () (const net_type & net) const;

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

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const port_type & p
                  , const std::string & direction
                  );
      }
    }
  }
}

#endif
