// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_USE_HPP
#define _XML_PARSE_TYPE_USE_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/util/parent.hpp>

#include <xml/parse/type/transition.fwd.hpp>

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
        ID_SIGNATURES(use);
        PARENT_SIGNATURES(transition);

      public:
        use_type ( ID_CONS_PARAM(use)
                 , PARENT_CONS_PARAM(transition)
                 , const std::string& name
                 );

        const std::string& name() const;

      private:
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
