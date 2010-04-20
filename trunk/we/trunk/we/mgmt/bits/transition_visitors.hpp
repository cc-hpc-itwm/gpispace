/*
 * =====================================================================================
 *
 *       Filename:  transition_visitors.hpp
 *
 *    Description:  implementation of visitors required by activity instances
 *
 *        Version:  1.0
 *        Created:  04/15/2010 12:31:04 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_TRANSITION_VISITORS_HPP
#define WE_MGMT_BITS_TRANSITION_VISITORS_HPP 1

#include <stdexcept>
#include <string>

#include <boost/variant.hpp>
#include <we/util/warnings.hpp>
#include <we/type/transition.hpp>

namespace we { namespace mgmt { namespace visitor {
  namespace exception
  {
    struct operation_not_supported
      : public std::runtime_error
    {
      operation_not_supported (const std::string & msg)
        : std::runtime_error (msg)
      { }
    };
  }

  class has_enabled 
    : public boost::static_visitor<bool>
  {
  public:
    template <typename Place, typename Trans, typename Edge, typename Token>
    bool operator () (const petri_net::net<Place, Trans, Edge, Token> & net) const
    {
      return ! net.enabled_transitions().empty();
    }

    template <typename T>
    bool operator () (const T &) const
    {
      throw exception::operation_not_supported ("has_enabled");
    }
  };

  template <typename Activity, typename OriginalInput>
  class input_mapper
    : public boost::static_visitor<>
  {
  private:
    typedef Activity activity_t;
    typedef OriginalInput input_t;

    activity_t & activity_;
    const input_t & original_input_;

  public:
    input_mapper ( activity_t & activity
                 , const input_t & input
                 )
    : activity_(activity)
    , original_input_ (input)
    {}

    template <typename Place, typename Trans, typename Edge, typename Token>
    void operator () (petri_net::net < Place
                                     , Trans
                                     , Edge
                                     , Token
                                     > & net )
    {
      // TODO beautify this
      for (typename input_t::const_iterator inp (original_input_.begin()); inp != original_input_.end(); ++inp)
      {
        typedef typename Activity::transition_type::port_id_t port_id_t;
        typedef typename Activity::transition_type::pid_t     pid_t;

        const port_id_t port_id  = activity_.transition().outer_to_inner (inp->second.first);
        const pid_t     place_id = activity_.transition().get_port (port_id).associated_place();

        // TODO work here
        activity_.input ().push_back
        (
          std::make_pair ( inp->first
                         , port_id
                         )
        );
        net.put_token ( place_id, inp->first );
      }
    }

    void operator () (we::type::module_call_t &)
    {
      // TODO beautify this
      for (typename input_t::const_iterator inp (original_input_.begin()); inp != original_input_.end(); ++inp)
      {
        typedef typename Activity::transition_type::port_id_t port_id_t;
        const port_id_t port_id  = activity_.transition().outer_to_inner (inp->second.first);

        // TODO work here
        activity_.input ().push_back
        (
          std::make_pair ( inp->first
                         , port_id
                         )
        );
      }
    }

    void operator () (we::type::expression_t &)
    {
      // TODO beautify this
      for (typename input_t::const_iterator inp (original_input_.begin()); inp != original_input_.end(); ++inp)
      {
        typedef typename Activity::transition_type::port_id_t port_id_t;
        const port_id_t port_id  = activity_.transition().outer_to_inner (inp->second.first);

        // TODO work here
        activity_.input ().push_back
        (
          std::make_pair ( inp->first
                         , port_id
                         )
        );
      }
    }
  };

  template<typename Activity, typename Engine>
  class activity_extractor
    : public boost::static_visitor<Activity>
  {
  private:
    Engine & engine_;
  public:
    explicit
    activity_extractor (Engine & engine)
      : engine_(engine)
    {}

    template <typename Place, typename Trans, typename Edge, typename Token>
    Activity operator () (petri_net::net < Place
                                         , Trans
                                         , Edge
                                         , Token
                                         > & net)
    {
      typedef petri_net::net < Place
                             , Trans
                             , Edge
                             , Token
                             > pnet_t;

      typedef typename pnet_t::activity_t activity_t;
      typedef typename pnet_t::input_t input_t;

      activity_t net_act = net.extract_activity_random (engine_);
      Activity act = Activity (net.get_transition (net_act.tid));

      input_mapper<Activity, input_t> input_mapper_visitor(act, net_act.input);
      boost::apply_visitor (input_mapper_visitor, act.transition().data());
      return act;
    }

    Activity operator () (we::type::module_call_t &)
    {
      throw exception::operation_not_supported ("activity_extractor (module_call)");
    }

    Activity operator () (we::type::expression_t &)
    {
      throw exception::operation_not_supported ("activity_extractor (expression)");
    }
  };

  template <typename Activity>
  class internal_executor
    : public boost::static_visitor<>
  {
  private:
    Activity & activity_;

  public:
    explicit
    internal_executor (Activity & activity)
      : activity_(activity)
    {}

    template <typename Place, typename Trans, typename Edge, typename Token>
    void operator () (petri_net::net < Place
                                         , Trans
                                         , Edge
                                         , Token
                                         > & net)
    {
      typedef petri_net::net < Place
                             , Trans
                             , Edge
                             , Token
                             > pnet_t;

      typedef typename Activity::input_t input_t;

      // TODO beautify this
      for (typename input_t::const_iterator inp (activity_.input().begin()); inp != activity_.input().end(); ++inp)
      {
        typedef typename Activity::transition_type::port_id_t port_id_t;
        typedef typename Activity::transition_type::pid_t     pid_t;

        const port_id_t port_id  = inp->second;
        const pid_t     place_id = activity_.transition().get_port (port_id).associated_place();
        net.put_token ( place_id, inp->first );
      }
    }

    void operator () (we::type::module_call_t &)
    {
      throw exception::operation_not_supported ("internal_executor (module_call)");
    }

    void operator () (we::type::expression_t & )
    {
      // construct context
      typedef expr::eval::context <signature::field_name_t> context_t;
      context_t context;

      typedef typename Activity::input_t input_t;
      typedef typename Activity::token_type token_type;
      typedef typename Activity::transition_type::port_id_t port_id_t;

      for (typename input_t::const_iterator top (activity_.input().begin()); top != activity_.input().end(); ++top)
      {
        const token_type token   = top->first;
        const port_id_t  port_id = top->second;

        token.bind ( we::type::detail::translate_port_to_name (activity_.transition(), port_id), context);
      }

      // evaluate
      // collect output
    }
  };
}}}

#endif
