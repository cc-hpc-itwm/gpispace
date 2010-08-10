// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_SIMPLE_PIPE_ELIMINATION_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_SIMPLE_PIPE_ELIMINATION_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/bits/transition/optimize/merge_places.hpp>

#include <stack>

namespace we { namespace type {
    namespace optimize
    {
      template<typename P, typename E, typename T>
      inline bool is_associated ( const transition_t<P, E, T> & trans
                                , const petri_net::pid_t & pid
                                , typename transition_t<P, E, T>::port_t & port
                                )
      {
        try
          {
            port = trans.get_port_by_associated_pid (pid);

            return true;
          }
        catch (const exception::port_undefined &)
          {
            return false;
          }
      }

      template<typename P, typename E, typename T>
      inline bool simple_pipe_elimination
      ( transition_t<P, E, T> & trans_parent
      , petri_net::net<P, transition_t<P,E,T>, E, T> & net
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef petri_net::net<P, transition_t, E, T> pnet_t;
        typedef typename pnet_t::transition_const_it transition_const_it;
        typedef petri_net::pid_t pid_t;
        typedef petri_net::tid_t tid_t;
        typedef typename transition_t::port_t port_t;

        bool modified (false);

        typedef std::stack<tid_t> stack_t;
        stack_t stack;

        for (transition_const_it t (net.transitions()); t.has_more(); ++t)
          {
            stack.push (*t);
          }

        while (!stack.empty())
          {
            const tid_t & tid (stack.top());

            transition_t trans (net.get_transition (tid));

            if ( (  boost::apply_visitor (content::visitor (), trans.data())
                  == content::expression
                  )
               && boost::get<expression_t> (trans.data()).is_empty()
               && trans.condition().is_const_true()
               )
              {
                if (  net.in_to_transition(tid).size() == 1
                   && net.out_of_transition(tid).size() == 1
                   )
                  { // one port only
                    const pid_t pid_in (*net.in_to_transition(tid));
                    const pid_t pid_out (*net.out_of_transition(tid));

                    if (  net.in_to_place (pid_out).size() == 1
                       || net.out_of_place (pid_in).size() == 1
                       )
                      {
                        port_t port_pid_in;
                        port_t port_pid_out;

                        const bool is_assoc_pid_in
                          (is_associated (trans_parent, pid_in, port_pid_in));
                        const bool is_assoc_pid_out
                          (is_associated (trans_parent, pid_out, port_pid_out));

                        if (  ! (  is_assoc_pid_in  && port_pid_in.is_input()
                                && is_assoc_pid_out && port_pid_out.is_input()
                                )
                           && ! (  is_assoc_pid_in  && port_pid_in.is_output()
                                && is_assoc_pid_out && port_pid_out.is_output()
                                )
                           )
                          {
                            net.delete_transition (tid);

                            merge_places (net, pid_in, pid_out);

                            if (is_assoc_pid_out)
                              {
                                trans_parent.UNSAFE_re_associate_port ( pid_out
                                                                      , pid_in
                                                                      );
                              }

                            modified = true;
                          }
                      }
                  }
                else
                  { // WORK HERE: more than one port
                  }
              }

            stack.pop();
          }

        return modified;
      }
    }
  }
}

#endif
