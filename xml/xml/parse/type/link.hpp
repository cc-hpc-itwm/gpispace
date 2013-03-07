// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_LINK_HPP
#define _XML_PARSE_TYPE_LINK_HPP

#include <xml/parse/type/link.fwd.hpp>

#include <string>

#include <fhg/util/xml.fwd.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct link_type
      {
      public:
        link_type (const std::string&);
        link_type (const std::string&, const std::string&);
        const std::string& href() const;
        const boost::optional<std::string>& prefix() const;
      private:
        std::string _href;
        boost::optional<std::string> _prefix;
      };

      bool operator== (const link_type&, const link_type&);

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const link_type & m
                  );
      }
    }
  }
}

#endif
