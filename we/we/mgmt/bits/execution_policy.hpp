#ifndef WE_MGMT_BITS_EXECUTION_POLICY_HPP
#define WE_MGMT_BITS_EXECUTION_POLICY_HPP 1

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
      template <typename Activity>
      struct execution_policy : public we::mgmt::context<int>
      {
        typedef typename we::mgmt::context<int>::result_type result_type;

        typedef Activity activity_t;
        typedef typename activity_t::id_t id_type;

        typedef petri_net::net net_t;
        typedef we::type::module_call_t mod_t;
        typedef we::type::expression_t expr_t;

        static const int EXTRACT = 0;
        static const int INJECT = 1;
        static const int EXTERNAL = 2;

        result_type handle_internally (activity_t &, net_t &)
        {
          return EXTRACT;
        }

        result_type handle_internally (activity_t &act, const mod_t & m)
        {
          return handle_externally (act, m);
        }

        result_type handle_internally (activity_t &, const expr_t &)
        {
          return INJECT;
        }

        result_type handle_externally (activity_t &, net_t &)
        {
          return EXTERNAL;
        }

        result_type handle_externally (activity_t &, const mod_t &)
        {
          return EXTERNAL;
        }

        result_type handle_externally (activity_t & act, const expr_t & e)
        {
          // print warning?
          return handle_internally ( act, e );
        }
      };
    }
  }
}

#endif
