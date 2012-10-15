// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_FWD_HPP
#define _XML_PARSE_TYPE_TRANSITION_FWD_HPP

#include <xml/parse/state.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

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

      // typedef xml::util::unique<connect_type>::elements_type connections_type;

      struct transition_type;

      template< typename Activity
              , typename Net
              , typename Trans
              , typename Fun
              , typename Map
              >
      void transition_synthesize
        ( const Trans & trans
        , const state::type & state
        , const Net & net
        , typename Activity::transition_type::net_type & we_net
        , const Map & pids
        , typename Activity::transition_type::edge_type & e
        );

      namespace dump
      {
        namespace visitor
        {
          class transition_dump;
        }
        inline void dump ( ::fhg::util::xml::xmlstream & s
                         , const transition_type & t
                         );
      }
    }
  }
}

#endif
