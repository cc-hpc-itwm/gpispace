// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
