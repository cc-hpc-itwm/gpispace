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
                                 , const id::transition& parent
                                 , const std::string& place
                                 , const std::string& port
                                 )
        : ID_INITIALIZE()
        , _parent (parent)
        , _place (place)
        , _port (port)
        , _name (_place + " <-> " + _port)
      {
        _id_mapper->put (_id, *this);
      }

      bool connect_type::has_parent() const
      {
        return _parent;
      }
      boost::optional<const transition_type&> connect_type::parent() const
      {
        return id_mapper()->get (_parent);
      }
      boost::optional<transition_type&> connect_type::parent()
      {
        return id_mapper()->get_ref (_parent);
      }

      const std::string& connect_type::place() const
      {
        return _place;
      }
      const std::string& connect_type::port() const
      {
        return _port;
      }
      const std::string& connect_type::name() const
      {
        return _name;
      }

      const std::string& connect_type::place (const std::string& place)
      {
        return _place = place;
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

          ::we::type::property::dump::dump (s, c.prop);

          s.close ();
        }
      }
    }
  }
}

