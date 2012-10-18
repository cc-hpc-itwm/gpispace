// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_FWD_HPP
#define _XML_PARSE_TYPE_TRANSITION_FWD_HPP

// #include <xml/util/unique.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      template<typename Fun>
        class transition_resolve;

      template<typename Fun>
        class transition_type_check;

      template<typename Fun>
        class transition_distribute_function;

      template<typename Fun>
        class transition_sanity_check;

      template<typename Fun>
        class transition_specialize;

      template<typename Net, typename Trans>
        class transition_get_function;

      // typedef xml::util::unique<connect_type,id::connect>::elements_type connections_type;

      struct transition_type;

      namespace dump
      {
        namespace visitor
        {
          class transition_dump;
        }
      }
    }
  }
}

#endif
