// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/bits/transition/optimize/option.hpp>
#include <we/type/bits/transition/optimize/simple_pipe_elimination.hpp>
#include <we/type/bits/transition/optimize/merge_expressions.hpp>

#include <stack>

#include <boost/foreach.hpp>
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
        class simplify_expression_sequences : public boost::static_visitor<bool>
        {
        private:
          boost::unordered_set<std::string> _outport_names;

        public:
          simplify_expression_sequences
            (const boost::unordered_set<std::string>& outport_names)
            : _outport_names (outport_names)
          {}

          bool operator () (module_call_t &) const { return false; }

          bool operator () (expression_t & expr) const
          {
            expr::parse::util::name_set_t needed_bindings;

            BOOST_FOREACH (std::string const& name, _outport_names)
            {
              expr::parse::util::name_set_t::value_type val;

              val.push_back (name);

              needed_bindings.insert (val);
            }

            return expr.simplify(needed_bindings);
          }

          bool operator ()
          (petri_net::net & net) const
          {
            typedef petri_net::net pnet_t;

            bool modified (false);

            typedef std::stack<petri_net::transition_id_type> stack_t;
            stack_t stack;

            BOOST_FOREACH ( const petri_net::transition_id_type& tid
                          , net.transitions() | boost::adaptors::map_keys
                          )
            {
              stack.push (tid);
            }

            while (!stack.empty())
              {
                const petri_net::transition_id_type tid (stack.top()); stack.pop();

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

        class optimize : public boost::static_visitor<bool>
        {
        private:
          const options::type & options;
          transition_t& trans_parent;

        public:
          optimize ( const options::type & _options
                   , transition_t & _trans_parent
                   )
            : options (_options)
            , trans_parent (_trans_parent)
          {}

          bool operator () (expression_t &) const { return false; }
          bool operator () (module_call_t &) const { return false; }

          bool operator ()
          (petri_net::net & net) const
          {
            typedef petri_net::net pnet_t;

            bool modified (false);

            modified |= (  options.simple_pipe_elimination()
                        && simple_pipe_elimination::run (trans_parent, net)
                        )
              ;
            modified |= (   options.merge_expressions()
                        && merge_expressions::run (trans_parent, net)
                        )
              ;

            typedef std::stack<petri_net::transition_id_type> stack_t;
            stack_t stack;

            BOOST_FOREACH ( const petri_net::transition_id_type& t
                          , net.transitions() | boost::adaptors::map_keys
                          )
            {
              stack.push (t);
            }

            while (!stack.empty())
              {
                const petri_net::transition_id_type tid (stack.top()); stack.pop();

                transition_t trans (net.get_transition (tid));

                const bool trans_modified
                  ( boost::apply_visitor ( optimize (options, trans)
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

      inline bool optimize (transition_t& t, const options::type & opts)
      {
        return
          boost::apply_visitor ( visitor::optimize (opts, t)
                               , t.data()
                               )
          | (  opts.simplify_expression_sequences()
            && boost::apply_visitor
               ( visitor::simplify_expression_sequences (t.port_names(PORT_OUT))
               , t.data()
               )
            )
          ;
      }
    } // namespace optimize
  } // namespace type
} // namespace we

#endif
