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
      connect_type::connect_type ( ID_CONS_PARAM(connect)
                                 , const std::string & _place
                                 , const std::string & _port
                                 , const id::transition& parent
                                 )
        : ID_INITIALIZE()
        , place (_place)
        , port (_port)
        , _name (_place + " <-> " + _port)
        , _parent (parent)
      {
        _id_mapper->put (_id, *this);
      }

      const id::transition& connect_type::parent() const
      {
        return _parent;
      }

      const std::string& connect_type::name() const
      {
        return _name;
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

