// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/use.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      use_type::use_type ( ID_CONS_PARAM(use)
                         , const id::transition& parent
                         , const std::string& name
                         )
        : ID_INITIALIZE()
        , _parent (parent)
        , _name (name)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& use_type::name() const
      {
        return _name;
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const use_type& u)
        {
          s.open ("use");
          s.attr ("name", u.name());
          s.close ();
        }
      }
    }
  }
}
