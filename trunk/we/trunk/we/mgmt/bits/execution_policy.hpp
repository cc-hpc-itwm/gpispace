#ifndef WE_MGMT_BITS_EXECUTION_POLICY_HPP
#define WE_MGMT_BITS_EXECUTION_POLICY_HPP 1

#include <we/util/show.hpp>

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
      struct execution_policy
      {
        typedef Activity activity_t;
        typedef typename activity_t::id_t id_type;

        typedef typename activity_t::transition_type::net_type net_t;
        typedef typename activity_t::transition_type::mod_type mod_t;
        typedef typename activity_t::transition_type::expr_type expr_t;

        typedef boost::function <void (id_type)> extractor_cb_type;
        typedef boost::function <void (id_type)> external_cb_type;
        typedef boost::function <void (id_type)> injector_cb_type;

        execution_policy ( extractor_cb_type extractor_cb
                         , external_cb_type external_cb
                         , injector_cb_type injector_cb
                         )
          : extractor (extractor_cb)
          , external (external_cb)
          , injector (injector_cb)
        { }

        void handle_internally (activity_t & act, net_t &)
        {
          act.inject_input ();
          extractor (act.id());
        }

        void handle_internally (activity_t &act, const mod_t & m)
        {
          handle_externally (act, m);
        }

        void handle_internally (activity_t & act, const expr_t &)
        {
          injector ( act.id() );
        }

        void handle_externally (activity_t & act, net_t &)
        {
          external (act.id());
        }

        void handle_externally (activity_t &act, const mod_t &)
        {
          external (act.id());
        }

        void handle_externally (activity_t & act, const expr_t & e)
        {
          // print warning?
          handle_internally ( act, e );
        }
      private:
        extractor_cb_type extractor;
        external_cb_type external;
        injector_cb_type injector;
      };
    }
  }
}

#endif
