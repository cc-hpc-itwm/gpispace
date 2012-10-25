// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/connect.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      connect_type::connect_type ( const std::string & _place
                                 , const std::string & _port
                                 , const id::connect& id
                                 , const id::transition& parent
                                 , id::mapper* id_mapper
                                 )
        : place (_place)
        , port (_port)
        , _name (_place + " <-> " + _port)
        , _id (id)
        , _parent (parent)
        , _id_mapper (id_mapper)
      {
        _id_mapper->put (_id, *this);
      }

      const id::connect& connect_type::id() const
      {
        return _id;
      }
      const id::transition& connect_type::parent() const
      {
        return _parent;
      }

      const std::string& connect_type::name() const
      {
        return _name;
      }

      bool connect_type::is_same (const connect_type& other) const
      {
        return id() == other.id() && parent() == other.parent();
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const connect_type & c
                  , const std::string & type
                  )
        {
          s.open ("connect-" + type);
          s.attr ("port", c.port);
          s.attr ("place", c.place);

          ::we::type::property::dump::dump (s, c.prop);

          s.close ();
        }
      }
    }
  }
}

