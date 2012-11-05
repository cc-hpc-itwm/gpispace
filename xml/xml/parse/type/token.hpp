// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TOKEN_HPP
#define _XML_PARSE_TYPE_TOKEN_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/util/parent.hpp>
#include <xml/parse/type/place.fwd.hpp>

#include <we/type/signature.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // misuse the signature type to read the token as recursive
      // string->string mapping, after the type of the place is
      // resolved, construct the concrete token out of this

      struct token_type : public signature::desc_t
      {
        ID_SIGNATURES(token);
        PARENT_SIGNATURES(place);

      public:
        token_type ( ID_CONS_PARAM(token)
                   , PARENT_CONS_PARAM(place)
                   , const signature::structured_t& structure
                   );

        token_type ( ID_CONS_PARAM(token)
                   , PARENT_CONS_PARAM(place)
                   , const std::string& value
                   );
      };
    }
  }
}

#endif
