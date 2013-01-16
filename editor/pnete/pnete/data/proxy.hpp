// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_PROXY_HPP
#define _PNETE_DATA_PROXY_HPP 1

#include <pnete/data/proxy.fwd.hpp>

#include <pnete/data/handle/expression.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/module.hpp>
#include <pnete/data/handle/net.hpp>
#include <pnete/data/internal.fwd.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>

#include <boost/variant.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace proxy
      {
        template<typename data_type, typename display_type>
        class proxy_base
        {
        public:
          proxy_base ( internal_type* root
                     , const handle::function& function
                     , data_type data
                     , display_type* display = NULL
                     )
            : _root (root)
            , _function (function)
            , _data (data)
            , _display (display)
          { }

          internal_type* root() const
          {
            return _root;
          }
          const handle::function& function() const
          {
            return _function;
          }

          const data_type& data() const
          {
            return _data;
          }
          display_type* display() const
          {
            return _display;
          }

        private:
          internal_type* _root;
          handle::function _function;

          data_type _data;
          display_type* _display;
        };

        typedef proxy_base<handle::expression> expression_proxy;
        typedef proxy_base<handle::module> mod_proxy;
        typedef proxy_base<handle::net, ui::graph::scene_type> net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;

        handle::function function (const type&);
        internal_type* root (const type&);
      }
    }
  }
}

#endif
