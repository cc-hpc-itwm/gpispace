// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/transition.hpp>

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
                                 , const we::type::property::type& properties
                                 )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _place (place)
        , _port (port)
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
      std::string connect_type::name() const
      {
        return _place + " <-> " + _port;
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

      id::ref::connect connect_type::clone
        (const boost::optional<parent_id_type>& parent) const
      {
        return connect_type ( id_mapper()->next_id()
                            , id_mapper()
                            , parent
                            , _place
                            , _port
                            , _properties
                            ).make_reference_id();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const connect_type & c
                  , const std::string & type
                  )
        {
          s.open ("connect-" + type);
          s.attr ("port", c.port());
          s.attr ("place", c.place());

          ::we::type::property::dump::dump (s, c.properties());

          s.close ();
        }
      }
    }
  }
}
