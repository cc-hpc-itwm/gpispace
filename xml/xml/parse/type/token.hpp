// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TOKEN_HPP
#define _XML_PARSE_TYPE_TOKEN_HPP

#include <we/type/signature.hpp>

#include <boost/filesystem.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // misuse the signature type to read the token as recursive
      // string->string mapping, after the type of the place is
      // resolved, construct the concrete token out of this

      typedef signature::desc_t token_type;
    }
  }
}

#endif
