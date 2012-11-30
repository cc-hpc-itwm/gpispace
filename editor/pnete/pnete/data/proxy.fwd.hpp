// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_DATA_PROXY_FWD_HPP
#define PNETE_DATA_PROXY_FWD_HPP

#include <pnete/data/internal.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>

#include <xml/parse/id/types.fwd.hpp>

#include <boost/variant/variant_fwd.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace proxy
      {
        template<typename data_type, typename display_type = void>
          class proxy_base;

        typedef proxy_base< ::xml::parse::id::ref::expression> expression_proxy;
        typedef proxy_base< ::xml::parse::id::ref::module> mod_proxy;
        typedef proxy_base< ::xml::parse::id::ref::net
                          , ui::graph::scene_type
                          > net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;
      }
    }
  }
}

#endif
