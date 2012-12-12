#ifndef WE_MGMT_BITS_EXECUTION_POLICY_HPP
#define WE_MGMT_BITS_EXECUTION_POLICY_HPP 1

#include <we/mgmt/type/activity.hpp>

#include <we/mgmt/context.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

namespace we
{
  namespace mgmt
  {
    namespace policy
    {
      /**
         The pupose of this class to implement an execution policy of a workflow
         management layer.

         external things will be delegated to an external execution function
         internal networks will be delegated to the extractor
         internal expressions will be executed directly and delegated to the injector process
       */
      struct execution_policy : public we::mgmt::context<int>
      {
        typedef we::mgmt::context<int>::result_type result_type;

        typedef we::mgmt::type::activity_t activity_t;

        typedef petri_net::net net_t;
        typedef we::type::module_call_t mod_t;
        typedef we::type::expression_t expr_t;

        static const int EXTRACT = 0;
        static const int INJECT = 1;
        static const int EXTERNAL = 2;

        execution_policy(){}

        result_type handle_internally (activity_t &, net_t &) const
        {
          return EXTRACT;
        }

        result_type handle_internally (activity_t &act, const mod_t & m) const
        {
          return handle_externally (act, m);
        }

        result_type handle_internally (activity_t &, const expr_t &) const
        {
          return INJECT;
        }

        result_type handle_externally (activity_t &, net_t &) const
        {
          return EXTERNAL;
        }

        result_type handle_externally (activity_t &, const mod_t &) const
        {
          return EXTERNAL;
        }

        result_type handle_externally (activity_t & act, const expr_t & e) const
        {
          // print warning?
          return handle_internally ( act, e );
        }
      };
    }
  }
}

#endif
