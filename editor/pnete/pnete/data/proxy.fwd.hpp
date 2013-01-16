// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_DATA_PROXY_FWD_HPP
#define PNETE_DATA_PROXY_FWD_HPP

#include <pnete/ui/graph/scene.fwd.hpp>

#include <pnete/data/handle/expression.fwd.hpp>
#include <pnete/data/handle/module.fwd.hpp>
#include <pnete/data/handle/net.fwd.hpp>

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

        typedef proxy_base<handle::expression> expression_proxy;
        typedef proxy_base<handle::module> mod_proxy;
        typedef proxy_base<handle::net, ui::graph::scene_type> net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;
      }
    }
  }
}

#endif
