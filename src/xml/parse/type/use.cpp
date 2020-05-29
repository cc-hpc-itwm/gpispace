#include <xml/parse/type/use.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      use_type::use_type ( const util::position_type& pod
                         , const std::string& name
                         )
        : with_position_of_definition (pod)
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
