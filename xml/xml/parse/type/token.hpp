// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TOKEN_HPP
#define _XML_PARSE_TYPE_TOKEN_HPP

#include <xml/parse/id/types.hpp>
#include <xml/parse/id/mapper.fwd.hpp>

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
      public:
        token_type ( const id::token& id
                   , const id::place& parent
                   , id::mapper* id_mapper
                   , const signature::structured_t& structure
                   );

        token_type ( const id::token& id
                   , const id::place& parent
                   , id::mapper* id_mapper
                   , const std::string& value
                   );

        const id::token& id() const;
        const id::place& parent() const;

      private:
        id::token _id;
        id::place _parent;
        id::mapper* _id_mapper;
      };
    }
  }
}

#endif
