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
         The purpose of this class to implement an execution policy of a workflow
         management layer.

         external things will be delegated to an external execution function
         internal networks will be delegated to the extractor
         internal expressions will be executed directly and delegated to the injector process
       */
      struct execution_policy : public we::mgmt::context
      {
        static const int EXTRACT = 0;
        static const int INJECT = 1;
        static const int EXTERNAL = 2;

        virtual int handle_internally (activity_t&, net_t&)
        {
          return EXTRACT;
        }

        virtual int handle_internally (activity_t& act, mod_t& m)
        {
          return handle_externally (act, m);
        }

        virtual int handle_internally (activity_t&, expr_t&)
        {
          return INJECT;
        }

        virtual int handle_externally (activity_t&, net_t&)
        {
          return EXTERNAL;
        }

        virtual int handle_externally (activity_t&, mod_t&)
        {
          return EXTERNAL;
        }

        virtual int handle_externally (activity_t& act, expr_t& e)
        {
          // print warning?
          return handle_internally (act, e);
        }
      };
    }
  }
}

#endif
