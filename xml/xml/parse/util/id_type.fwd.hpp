// bernd.loerwald@itwm.fraunhofer.de

#ifndef XML_UTIL_ID_TYPE_FWD_HPP
#define XML_UTIL_ID_TYPE_FWD_HPP

#include <boost/cstdint.hpp>
#include <boost/functional/hash.hpp>

namespace xml
{
  namespace parse
  {
    namespace id
    {
#define MAKE_ID(name)                                                   \
      struct name

      MAKE_ID (connect);
      MAKE_ID (expression);
      MAKE_ID (function);
      MAKE_ID (module);
      MAKE_ID (net);
      MAKE_ID (place);
      MAKE_ID (place_map);
      MAKE_ID (port);
      MAKE_ID (specialize);
      MAKE_ID (structure);
      MAKE_ID (tmpl);
      MAKE_ID (token);
      MAKE_ID (transition);
      MAKE_ID (use);

#undef MAKE_ID
    }
  }
}

#endif
