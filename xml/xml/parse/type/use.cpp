// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/use.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      use_type::use_type ( const id::use& id
                         , const id::transition& parent
                         , const std::string& name
                         )
        : _id (id)
        , _parent (parent)
        , _name (name)
      {}

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
