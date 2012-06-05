// mirko.rahn@itwm.fraunhofer.de

#ifndef _NET_WITH_TRANSITION_FUNCTION_HPP
#define _NET_WITH_TRANSITION_FUNCTION_HPP

#include <we/net.hpp>

namespace petri_net
{
  template<typename Place, typename Transition, typename Edge, typename Token>
  class net_with_transition_function
    : public net<Place, Transition, Edge, Token>
  {
  private:
    typedef net<Place, Transition, Edge, Token> super;

    typedef Function::Transition::Traits<Token> tf_traits;
    typedef typename tf_traits::fun_t trans_t;
    typedef boost::unordered_map<tid_t, trans_t> trans_map_t;

    trans_map_t trans;

  public:
    net_with_transition_function ( const std::string & _name = "noname"
                                 , const pid_t & _places = 10
                                 , const tid_t & _transitions = 10
                                 )
      : super (_name, _places, _transitions)
      , trans ()
    {}

    void
    set_transition_function (const tid_t & tid, const trans_t & f)
    {
      trans[tid] = f;
    }

    typename super::output_t
    run_activity (const typename super::activity_t & activity) const
    {
      const typename trans_map_t::const_iterator f (trans.find (activity.tid));

      return (f == trans.end())
        ? Function::Transition::Default<Token>() ( activity.input
                                                 , activity.output_descr
                                                 )
        : f->second ( activity.input
                    , activity.output_descr
                    )
        ;
    }

    tid_t fire (const tid_t & tid)
    {
      const typename super::activity_t activity (super::extract_activity (tid));
      const typename super::output_t output (run_activity (activity));
      inject_activity_result (output);
      return tid;
    }

    tid_t fire_first ()
    {
      return fire (super::enabled_transitions().first());
    }

    template<typename Engine>
    tid_t fire_random (Engine & engine)
    {
      return fire (super::enabled_transitions().random (engine));
    }
  };
} // namespace petri_net

#endif // _NET_WITH_TRANSITION_FUNCTION_HPP
