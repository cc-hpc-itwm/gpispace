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

#include <fhg/util/maybe.hpp>

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
        public:
          bool operator () (module_call_t &) const { return false; }

          bool operator () (expression_t & expr) const
          {
            return expr.simplify();
          }

          template<typename P, typename E, typename T>
          bool operator ()
          (petri_net::net<P, transition_t<P,E,T>, E, T> & net) const
          {
            typedef transition_t<P, E, T> transition_t;
            typedef petri_net::net<P, transition_t, E, T> pnet_t;
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
                  (boost::apply_visitor (*this, trans.data()));

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

          template<typename P, typename E, typename T>
          bool operator ()
          (petri_net::net<P, transition_t<P,E,T>, E, T> & net) const
          {
            typedef transition_t<P, E, T> transition_t;
            typedef petri_net::net<P, transition_t, E, T> pnet_t;
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

      template<typename P, typename E, typename T>
      inline bool optimize (transition_t<P,E,T> & t, const options::type & opts)
      {
        typedef transition_t<P, E, T> transition_t;

        return
          boost::apply_visitor
          ( visitor::optimize<transition_t> (opts, t)
          , t.data()
          )
          |
          (  opts.simplify_expression_sequences()
          && boost::apply_visitor
             ( visitor::simplify_expression_sequences<transition_t>()
             , t.data()
             )
          )
          ;
      }
    } // namespace optimize
  } // namespace type
} // namespace we

#endif
