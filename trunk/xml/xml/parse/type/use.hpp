// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_USE_HPP
#define _XML_PARSE_TYPE_USE_HPP

#include <xml/parse/types.hpp>

#include <iostream>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // ******************************************************************* //

      struct use_type
      {
        std::string name;

        explicit use_type (const std::string & _name)
          : name (_name)
        {}
      };

      namespace dump
      {
        void inline dump (xml_util::xmlstream & s, const use_type & u)
        {
          s.open ("use");
          s.attr ("name", u.name);
          s.close ();
        }
      }
    }
  }
}

#endif
