#pragma once

#include <we/type/net.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>

#include <we/type/activity.fwd.hpp>

namespace we
{
    /* context requirements

       net:
       inject tokens into net
       ctxt.handle_internally (act, net) or handle_externally depending on is_internal
       -> extractor

       expr:
       evaluate expression
       -> injector

       mod:
       prepare input
       [(token-on-place)], { place <-> name }
       ctxt.handle_externally (act, mod_call_t)

    */

    class context
    {
    protected:
      typedef we::type::activity_t activity_t;
      typedef we::type::net_type net_t;
      typedef we::type::module_call_t mod_t;
      typedef we::type::expression_t expr_t;

    public:
      virtual void handle_internally (activity_t&, net_t const&) = 0;
      virtual void handle_externally (activity_t&, net_t const&) = 0;
      virtual void handle_externally (activity_t&, mod_t const&) = 0;

      virtual ~context() = default;
    };
}
