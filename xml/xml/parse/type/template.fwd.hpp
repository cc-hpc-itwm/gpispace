// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TEMPLATE_FWD_HPP
#define _XML_PARSE_TYPE_TEMPLATE_FWD_HPP 1

#include <xml/parse/state.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct template_type;

      namespace dump
      {
        void dump (fhg::util::xml::xmlstream & s, const template_type & t);
      }
    }
  }
}

#endif
