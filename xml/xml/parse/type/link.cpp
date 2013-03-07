// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/type/link.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      link_type::link_type (const std::string& href)
        : _href (href)
        , _prefix (boost::none)
      {}
      link_type::link_type ( const std::string& href
                           , const std::string& prefix
                           )
        : _href (href)
        , _prefix (prefix)
      {}
      const std::string& link_type::href() const
      {
        return _href;
      }
      const boost::optional<std::string>& link_type::prefix() const
      {
        return _prefix;
      }

      bool operator== (const link_type& a, const link_type& b)
      {
        return a.href() == b.href() && a.prefix() == b.prefix();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const link_type& l)
        {
          s.open ("link");
          s.attr ("href", l.href());
          s.attr ("prefix", l.prefix());
          s.close();
        }
      }
    }
  }
}
