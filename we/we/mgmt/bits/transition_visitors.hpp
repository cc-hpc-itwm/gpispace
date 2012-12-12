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
#include <we/type/net.fwd.hpp>

namespace we
{
  namespace mgmt
  {
    namespace visitor
    {
      template <typename Activity, typename Context>
      class executor
        : public boost::static_visitor<typename Context::result_type>
      {
      private:
        Activity& _activity;
        Context& _ctxt;

      public:
        typedef typename Context::result_type result_type;

        executor (Activity & activity, Context & ctxt)
          : _activity (activity)
          , _ctxt (ctxt)
        {}

        result_type operator () (petri_net::net& net) const
        {
          if (_activity.transition().is_internal())
            {
              return _ctxt.handle_internally (_activity, net);
            }
          else
            {
              return _ctxt.handle_externally (_activity, net);
            }
        }

        result_type operator() (we::type::module_call_t & mod) const
        {
          return _ctxt.handle_externally (_activity, mod);
        }

        result_type operator() (we::type::expression_t & expr) const
        {
          expr::eval::context context;

          for ( typename Activity::input_t::const_iterator top
                  (_activity.input().begin())
              ; top != _activity.input().end()
              ; ++top
              )
            {
              context.bind ( _activity.transition().name_of_port (top->second)
                           , top->first.value
                           );
            }

          expr.ast ().eval_all (context);

          for ( we::type::transition_t::const_iterator port_it
                  (_activity.transition().ports_begin())
              ; port_it != _activity.transition().ports_end()
              ; ++port_it
              )
            {
              if (port_it->second.is_output())
                {
                  _activity.add_output
                    ( typename Activity::output_t::value_type
                      ( token::type ( port_it->second.name()
                                    , port_it->second.signature()
                                    , context
                                    )
                      , port_it->first
                      )
                    );
                }
            }

          return _ctxt.handle_internally (_activity, expr);
        }
      };
    }
  }
}

#endif
