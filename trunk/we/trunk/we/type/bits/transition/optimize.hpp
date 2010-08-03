// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_HPP 1

#include <we/type/transition.hpp>

#include <stack>

#include <boost/program_options.hpp>

#include <fhg/util/read_bool.hpp>

namespace we { namespace type {
    namespace optimize
    {
      // ******************************************************************* //

      namespace options
      {
        namespace po = boost::program_options;
        namespace property = we::type::property;

        struct type
        {
        private:
          bool _not;
          bool _simple_pipe_elimination;

          std::string _Onot;
          std::string _Osimple_pipe_elimination;

        public:
          type (void)
            : _not (false)
            , _simple_pipe_elimination (true)

            , _Onot ("Onot")
            , _Osimple_pipe_elimination ("Osimple-pipe-elimination")
          {}

          // *************************************************************** //

          bool property ( const property::path_type & path
                        , const property::value_type & value
                        )
          {
            if (path.size() != 1)
              {
                return false;
              }

#define GET_PROP(x)                                                       \
            else if (path.size() == 1 && path[0] == _O ## x)              \
              {                                                           \
                _ ## x = fhg::util::read_bool (value); return true;       \
              }

            GET_PROP(simple_pipe_elimination)

#undef GET_PROP
            else
              {
                return false;
              }
          }

          // *************************************************************** //

#define ACCESS(x)                                         \
        bool x (void) const { return (!_not) && _ ## x; } \
        bool & x (void) { return _ ## x; }

        ACCESS(simple_pipe_elimination)
#undef ACCESS

          // *************************************************************** //

          void add_options (po::options_description & desc)
          {
#define VAL(x) po::value<bool>(&_ ## x)->default_value (_ ## x)

            desc.add_options ()
              ( _Onot.c_str()
              , VAL(not)
              , "disable all optimizations"
              )
              ( _Osimple_pipe_elimination.c_str()
              , VAL(simple_pipe_elimination)
              , "eliminate simple pipeline transitions"
              )
              ;
#undef VAL
          }
        };
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void merge_places
      ( petri_net::net<P, transition_t<P,E,T>, E, T> & net
      , const petri_net::pid_t & pid_A
      , const petri_net::pid_t & pid_B
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef petri_net::net<P, transition_t, E, T> pnet_t;
        typedef typename pnet_t::transition_const_it transition_const_it;
        typedef petri_net::pid_t pid_t;
        typedef petri_net::eid_t eid_t;
        typedef petri_net::tid_t tid_t;
        typedef petri_net::connection_t connection_t;
        typedef petri_net::adj_transition_const_it adj_transition_const_it;

        typedef std::pair<tid_t,eid_t> pair_t;
        typedef std::stack<pair_t> stack_t;
        stack_t stack;

        // rewire pid_B -> trans to pid_A -> trans
        for ( adj_transition_const_it trans_out_B (net.out_of_place (pid_B))
            ; trans_out_B.has_more()
            ; ++trans_out_B
            )
          {
            stack.push (std::make_pair (*trans_out_B, trans_out_B()));
          }

        while (!stack.empty())
          {
            const stack_t::value_type & pair (stack.top());
            const tid_t & tid_trans_out_B (pair.first);
            const eid_t & eid_out_B (pair.second);

            const E edge (net.get_edge (eid_out_B));
            connection_t connection (net.get_edge_info (eid_out_B));

            net.delete_edge (eid_out_B);

            connection.pid = pid_A;

            net.add_edge (edge, connection);

            transition_t trans_out_B (net.get_transition (tid_trans_out_B));

            typename transition_t::port_id_with_prop_t port_id_with_prop
              (trans_out_B.input_port_by_pid (pid_B));

            trans_out_B.re_connect_outer_to_inner
              ( pid_B
              , pid_A
              , port_id_with_prop.first
              , port_id_with_prop.second
              );

            net.modify_transition (tid_trans_out_B, trans_out_B);

            stack.pop();
          }

        // rewire trans -> pid_B to trans -> pid_A
        for ( adj_transition_const_it trans_in_B (net.in_to_place (pid_B))
            ; trans_in_B.has_more()
            ; ++trans_in_B
            )
          {
            stack.push (std::make_pair (*trans_in_B, trans_in_B()));
          }

        while (!stack.empty())
          {
            const stack_t::value_type & pair (stack.top());
            const tid_t & tid_trans_in_B (pair.first);
            const eid_t & eid_in_B (pair.second);

            const E edge (net.get_edge (eid_in_B));
            connection_t connection (net.get_edge_info (eid_in_B));

            net.delete_edge (eid_in_B);

            connection.pid = pid_A;

            net.add_edge (edge, connection);

            transition_t trans_in_B (net.get_transition (tid_trans_in_B));

            typename transition_t::port_id_with_prop_t port_id_with_prop
              (trans_in_B.output_port_by_pid (pid_B));
            
            trans_in_B.re_connect_inner_to_outer
              ( port_id_with_prop.first
              , pid_A
              , port_id_with_prop.second
              );

            net.modify_transition (tid_trans_in_B, trans_in_B);

            stack.pop();
          }

        net.delete_place (pid_B);
      }

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
                            net.delete_edge (net.in_to_transition(tid)());
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

      namespace visitor
      {
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

            modified |= (  options.simple_pipe_elimination ()
                        &&         simple_pipe_elimination (trans_parent, net)
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
      }

      template<typename P, typename E, typename T>
      inline bool optimize (transition_t<P,E,T> & t, const options::type & opts)
      {
        typedef transition_t<P, E, T> transition_t;

        return boost::apply_visitor ( visitor::optimize<transition_t> (opts, t)
                                    , t.data()
                                    );
      }
    }
  }
}

#endif
