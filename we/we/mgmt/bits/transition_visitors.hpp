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

#include <fhg/util/show.hpp>

#include <we/type/transition.hpp>
#include <we/net.hpp>
#include <we/type/id.hpp>

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

  template <typename Activity>
  class output_collector
    : public boost::static_visitor<>
  {
  private:
    Activity & activity_;

  public:
    output_collector (Activity & activity)
      : activity_(activity)
    {}

    void operator () (petri_net::net& net)
    {
      typedef petri_net::net pnet_t;

      typedef typename Activity::output_t output_t;
      typedef we::type::transition_t::const_iterator port_iterator;

      // collect output
      for ( port_iterator port_it (activity_.transition().ports_begin())
          ; port_it != activity_.transition().ports_end()
          ; ++port_it
          )
      {
        if (port_it->second.is_output())
        {
          if (port_it->second.has_associated_place())
          {
            const petri_net::port_id_type port_id = port_it->first;
            const petri_net::place_id_type pid = port_it->second.associated_place();

            for ( typename pnet_t::token_place_it top ( net.get_token (pid) )
                ; top.has_more ()
                ; ++top
                )
            {
              activity_.add_output (typename output_t::value_type (*top, port_id));
            }
            net.delete_all_token ( pid );
          }
          else
          {
            throw std::runtime_error ( "output port ("
                                       + fhg::util::show (port_it->first)
                                       + ", " + fhg::util::show (port_it->second) + ") "
                                     + "is not associated with any place!"
                                     );
          }
        }
      }
    }

    void operator () (const we::type::module_call_t &) const
    {
      return; // nothting todo, activity already contains output
    }

    void operator () (const we::type::expression_t &) const
    {
      return; // nothting todo, activity already contains output
    }

    /*
    template <typename T>
    void operator () ( T & )
    {
      throw exception::operation_not_supported ("output_collector (unknown T)");
    }
    */
  };

  template <typename Net, typename Transition, typename Output>
  void inject_output_to_net ( Net & net, Transition const & trans, Output const & output)
  {
    // iterate over output of child
    for ( typename Output::const_iterator top (output.begin())
        ; top != output.end()
        ; ++top
        )
    {
      try
      {
        token::put ( net
                   , trans.inner_to_outer ( top->second )
                   , top->first
                   );
      } catch ( const we::type::exception::not_connected <petri_net::port_id_type> &)
      {
        std::cerr << "W: transition generated output, but port is not connected:"
                  << " trans=\"" << trans.name() << "\""
                  << " port="
                  << trans.name_of_port (top->second)
                  << "(" << top->second << ")"
                  << " token=" << fhg::util::show (top->first)
                  << std::endl;
      }
    }
  }

  template <typename Net, typename Transition, typename Input>
  void inject_input_to_net ( Net & net, Transition & trans, const Input & input)
  {
    for (typename Input::const_iterator inp (input.begin()); inp != input.end(); ++inp)
    {
      const petri_net::port_id_type port_id (inp->second);

      if (trans.get_port (port_id).has_associated_place())
        {
          const petri_net::place_id_type place_id
            (trans.get_port (port_id).associated_place());

          token::put (net, place_id, inp->first);
        }
    }
  }

  template <typename Activity>
  class injector
    : public boost::static_visitor<void>
  {
  private:
    typedef typename Activity::input_t input_t;

    Activity & activity_;
    input_t const & input_;

  public:
    explicit
    injector (Activity & activity, input_t const & input)
      : activity_(activity)
      , input_(input)
    {}

    void operator () (petri_net::net& net)
    {
      inject_input_to_net (net, activity_.transition(), input_);
    }

    void operator () (const we::type::module_call_t & )
    {
    }

    void operator () (const we::type::expression_t & )
    {
    }
  };

  template <typename Activity, typename Context>
  class executor
    : public boost::static_visitor<typename Context::result_type>
  {
  private:
    Activity & activity_;
    Context & ctxt_;
    const bool internal_;

  public:
    typedef typename Context::result_type result_type;

    explicit
    executor (Activity & activity, Context & ctxt)
      : activity_(activity)
      , ctxt_(ctxt)
      , internal_(activity.transition().is_internal())
    {}

    result_type operator () (petri_net::net& net)
    {
      if (internal_)
      {
        return ctxt_.handle_internally ( activity_, net );
      }
      else
      {
        return ctxt_.handle_externally ( activity_, net );
      }
    }

    result_type operator () (const we::type::module_call_t & mod)
    {
      return ctxt_.handle_externally ( activity_, mod );
    }

    result_type operator () (const we::type::expression_t & expr)
    {
      // construct context
      typedef expr::eval::context context_t;
      context_t context;

      typedef typename Activity::input_t input_t;
      typedef typename Activity::output_t output_t;
      typedef we::type::transition_t::const_iterator port_iterator;

      for ( typename input_t::const_iterator top (activity_.input().begin())
          ; top != activity_.input().end()
          ; ++top
          )
      {
        const token::type token = top->first;
        const petri_net::port_id_type port_id = top->second;

        context.bind
          (activity_.transition().name_of_port (port_id), token.value);
      }

      // evaluate
      expr.ast ().eval_all (context);

      // collect output
      for ( port_iterator port_it (activity_.transition().ports_begin())
          ; port_it != activity_.transition().ports_end()
          ; ++port_it
          )
      {
        if (port_it->second.is_output())
        {
          const petri_net::port_id_type port_id = port_it->first;
          const token::type token ( port_it->second.name()
                                 , port_it->second.signature()
                                 , context
                                 );
          activity_.add_output (typename output_t::value_type (token, port_id));
        }
      }

      return ctxt_.handle_internally ( activity_, expr );
    }
  };
}}}

#endif
