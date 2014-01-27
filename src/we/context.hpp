#ifndef WE_MGMT_CONTEXT_HPP
#define WE_MGMT_CONTEXT_HPP 1

#include <we/type/net.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>

#include <we/type/activity.fwd.hpp>

namespace we
{
    /* context requirements

       internal
       ========

       net:
       inject tokens into net
       ctxt.handle_internally (act, net)
       -> extractor

       expr:
       evaluate expression
       ctxt.handle_internally (act, expr)
       -> injector

       mod:
       prepare input
       [(token-on-place)], { place <-> name }
       ctxt.handle_internally (act, mod_call_t)

       external
       ========

       ctxt.handle_externally (act, net)
       ctxt.handle_externally (act, expr)
       ctxt.handle_externally (act, mod)

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
      virtual void handle_internally (activity_t&, mod_t const&) = 0;
      virtual void handle_internally (activity_t&, expr_t const&) = 0;
      virtual void handle_externally (activity_t&, net_t const&) = 0;
      virtual void handle_externally (activity_t&, mod_t const&) = 0;
      virtual void handle_externally (activity_t&, expr_t const&) = 0;

      virtual ~context() {}
    };
}

#endif
