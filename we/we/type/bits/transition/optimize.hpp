// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/bits/transition/optimize/option.hpp>
#include <we/type/bits/transition/optimize/simple_pipe_elimination.hpp>
#include <we/type/bits/transition/optimize/merge_expressions.hpp>

#include <stack>

#include <boost/program_options.hpp>
#include <boost/variant.hpp>

#include <we/expr/parse/util/get_names.hpp>

namespace we
{
  namespace type
  {
    namespace optimize
    {
      namespace visitor
      {
        template<typename Trans>
        class simplify_expression_sequences : public boost::static_visitor<bool>
        {
        private:
          typedef typename Trans::port_names_t port_names_t;
          port_names_t _outport_names;

        public:
          simplify_expression_sequences (const port_names_t & outport_names)
            : _outport_names (outport_names)
          {}

          bool operator () (module_call_t &) const { return false; }

          bool operator () (expression_t & expr) const
          {
            expr::parse::util::name_set_t needed_bindings;

            for ( typename port_names_t::const_iterator name (_outport_names.begin())
                ; name != _outport_names.end()
                ; ++name
                )
              {
                expr::parse::util::name_set_t::value_type val;

                val.push_back (*name);

                needed_bindings.insert (val);
              }

            return expr.simplify(needed_bindings);
          }

          template<typename E>
          bool operator ()
          (petri_net::net<transition_t<E>, E> & net) const
          {
            typedef transition_t<E> transition_t;
            typedef petri_net::net<transition_t, E> pnet_t;
            typedef typename pnet_t::transition_const_it transition_const_it;
            typedef petri_net::tid_t tid_t;

            bool modified (false);

            typedef std::stack<tid_t> stack_t;
            stack_t stack;

            for (transition_const_it t (net.transitions()); t.has_more(); ++t)
              {
                stack.push (*t);
              }

            while (!stack.empty())
              {
                const tid_t tid (stack.top()); stack.pop();

                transition_t trans (net.get_transition (tid));

                const bool trans_modified
                  ( boost::apply_visitor
                    ( simplify_expression_sequences(trans.port_names(PORT_OUT))
                    , trans.data()
                    )
                  );

                if (trans_modified)
                  {
                    net.modify_transition (tid, trans);
                  }

                modified |= trans_modified;
              }

            return modified;
          }
        };

        template<typename Trans>
        class optimize : public boost::static_visitor<bool>
        {
        private:
          const options::type & options;
          Trans & trans_parent;

        public:
          optimize ( const options::type & _options
                   , Trans & _trans_parent
                   )
            : options (_options)
            , trans_parent (_trans_parent)
          {}

          bool operator () (expression_t &) const { return false; }
          bool operator () (module_call_t &) const { return false; }

          template<typename E>
          bool operator ()
          (petri_net::net<transition_t<E>, E> & net) const
          {
            typedef transition_t<E> transition_t;
            typedef petri_net::net<transition_t, E> pnet_t;
            typedef typename pnet_t::transition_const_it transition_const_it;
            typedef petri_net::tid_t tid_t;

            bool modified (false);

            modified |= (  options.simple_pipe_elimination()
                        && simple_pipe_elimination::run (trans_parent, net)
                        )
              ;
            modified |= (   options.merge_expressions()
                        && merge_expressions::run (trans_parent, net)
                        )
              ;

            typedef std::stack<tid_t> stack_t;
            stack_t stack;

            for (transition_const_it t (net.transitions()); t.has_more(); ++t)
              {
                stack.push (*t);
              }

            while (!stack.empty())
              {
                const tid_t tid (stack.top()); stack.pop();

                transition_t trans (net.get_transition (tid));

                const bool trans_modified
                  ( boost::apply_visitor ( optimize<Trans>(options, trans)
                                         , trans.data()
                                         )
                  );

                if (trans_modified)
                  {
                    net.modify_transition (tid, trans);
                  }

                modified |= trans_modified;
              }

            return modified;
          }
        };
      } // namespace visitor

      template<typename E>
      inline bool optimize (transition_t<E> & t, const options::type & opts)
      {
        typedef transition_t<E> transition_t;

        return
          boost::apply_visitor
          ( visitor::optimize<transition_t> (opts, t)
          , t.data()
          )
          |
          (  opts.simplify_expression_sequences()
          && boost::apply_visitor
             ( visitor::simplify_expression_sequences<transition_t>
               (t.port_names(PORT_OUT))
             , t.data()
             )
          )
          ;
      }
    } // namespace optimize
  } // namespace type
} // namespace we

#endif
