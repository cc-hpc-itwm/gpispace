// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_FWD_HPP
#define _XML_PARSE_TYPE_NET_FWD_HPP

// #include <string>
// #include <boost/unordered/unordered_map_fwd.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      class function_with_mapping_type;

      struct net_type;

      // inline net_type
      //   set_prefix (const net_type & net_old, const std::string & prefix);

      // template <typename Activity, typename Net, typename Fun>
      //   boost::unordered_map< std::string
      //                       , typename Activity::transition_type::pid_t
      //                       >
      //   net_synthesize ( typename Activity::transition_type::net_type & we_net
      //                  , const place_map_map_type & place_map_map
      //                  , const Net & net
      //                  , const state::type & state
      //                  , typename Activity::transition_type::edge_type & e
      //                  );
    }
  }
}

#endif
