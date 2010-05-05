#ifndef WE_MGMT_BITS_EXECUTION_POLICY_HPP
#define WE_MGMT_BITS_EXECUTION_POLICY_HPP 1

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

        typedef typename activity_t::transition_type::net_type net_t;
        typedef typename activity_t::transition_type::mod_type mod_t;
        typedef typename activity_t::transition_type::expr_type expr_t;

        void handle_internally (activity_t &, net_t &)
        {
          /*
          act.inject_input ();

          while (act.has_enabled())
          {
            we::activity_t sub = act.extract ();
            sub.execute (*this);
            act.inject (sub);
          }

          act.collect_output ();
          */

          // act.inject_input ();
          // post_activity_notification (act.id());
          // return;
        }

        void handle_internally (activity_t &, const mod_t &)
        {
          // throw exception
        }

        void handle_internally (activity_t & , const expr_t & )
        {
          // inform injector to inject
          // act.collect_output()
          // parent_of (act.id()).inject (act)
          // nothing to do
        }

        void handle_externally (activity_t &, net_t &)
        {
          /*
          we::activity_t result ( we::util::text_codec::decode<we::activity_t> (fake_external (we::util::text_codec::encode(act), n)));
          act = result;
          */
        }

        void handle_externally (activity_t &, const mod_t &)
        {
          /*
          we::activity_t result ( we::util::text_codec::decode<we::activity_t> (fake_external (we::util::text_codec::encode(act), module_call)));
          act = result;
          */
        }

        void handle_externally (activity_t & act, const expr_t & e)
        {
          // print warning?
          handle_internally ( act, e );
        }
      };
    }
  }
}

#endif
