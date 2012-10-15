// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_FWD_HPP
#define _XML_PARSE_TYPE_PLACE_FWD_HPP

#include <fhg/util/xml.fwd.hpp>

// #include <list>
// #include <xml/util/unique>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      class default_construct_value;
      class construct_value;

      // typedef std::list<value::type> values_type;
      // typedef std::list<token_type> tokens_type;

      struct place_type;

      // typedef xml::util::unique<place_type>::elements_type places_type;

      namespace dump
      {
        inline void dump (::fhg::util::xml::xmlstream&, const place_type&);
      }
    }
  }
}

#endif
