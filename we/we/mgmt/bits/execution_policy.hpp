#ifndef WE_MGMT_BITS_EXECUTION_POLICY_HPP
#define WE_MGMT_BITS_EXECUTION_POLICY_HPP 1

#include <we/mgmt/context.hpp>

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

        typedef typename activity_t::transition_type::net_type net_t;
        typedef typename activity_t::transition_type::mod_type mod_t;
        typedef typename activity_t::transition_type::expr_type expr_t;

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
