// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_USE_HPP
#define _XML_PARSE_TYPE_USE_HPP

#include <xml/parse/type/id.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct use_type
      {
        ID_SIGNATURES(use)

      public:
        use_type ( ID_CONS_PARAM(use)
                 , const id::transition& parent
                 , const std::string& name
                 );

        const std::string& name() const;

      private:
        id::transition _parent;

        std::string _name;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const use_type& u);
      }
    }
  }
}

#endif
