// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <vector>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct transition
      {
        typedef std::vector<connect> connect_vec_type;

        connect_vec_type in;
        connect_vec_type out;
        connect_vec_type read;

        // WORK HERE: mutal exclusion not ensured by xsd scheme
        maybe<function> f;
        maybe<std::string> use;

        std::string name;
      };
    }
  }
}

#endif
