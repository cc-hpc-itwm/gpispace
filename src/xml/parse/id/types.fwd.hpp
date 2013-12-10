// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_PARSE_ID_TYPES_FWD_HPP
#define XML_PARSE_ID_TYPES_FWD_HPP

namespace xml
{
  namespace parse
  {
    namespace id
    {
#define ITEM(NAME,__IGNORE,__IGNORE2,__IGNORE3)                         \
      struct NAME;

#include <xml/parse/id/helper.lst>

#undef ITEM

      namespace ref
      {
#define ITEM(NAME,__IGNORE,__IGNORE2,__IGNORE3)                         \
        struct NAME;

#include <xml/parse/id/helper.lst>

#undef ITEM
      }
    }
  }
}

#endif
