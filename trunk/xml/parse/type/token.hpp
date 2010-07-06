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
      struct token
      {
      public:
        // misuse the signature type to read the token as recursive
        // string->string mapping, after the type of the place is
        // resolved, construct the concrete token out of this

        signature::desc_t content;
        boost::filesystem::path path;

        token ( const signature::desc_t & _content
              , const boost::filesystem::path & _path
              )
          : content (_content)
          , path (_path)
        {}
      };

      std::ostream & operator << (std::ostream & s, const token & tok)
      {
        return s << "token ("
                 << "content = " << tok.content
                 << ", path = " << tok.path
                 << ")"
          ;
      }
    }
  }
}

#endif
