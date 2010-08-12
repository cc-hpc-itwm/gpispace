// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_EXPRESSIONS_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_EXPRESSIONS_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>

#include <we/type/bits/transition/optimize/is_associated.hpp>

#include <fhg/util/maybe.hpp>
#include <rewrite/validprefix.hpp>

#include <stack>

namespace we { namespace type {
    namespace optimize { namespace merge_expressions
    {
      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline fhg::util::maybe<std::pair< const transition_t<P, E, T> &
                                       , const petri_net::tid_t &
                                       >
                             >
      expression_predecessor
      ( const transition_t<P, E, T> & trans
      , const petri_net::tid_t & tid
      , const petri_net::net<P, transition_t<P, E, T>, E, T> & net
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef petri_net::net<P, transition_t, E, T> pnet_t;
        typedef petri_net::adj_place_const_it adj_place_const_it;
        typedef petri_net::adj_transition_const_it adj_transition_const_it;
        typedef petri_net::tid_t tid_t;
        typedef typename transition_t::const_iterator const_iterator;

        typedef std::pair<const transition_t &, const tid_t &> pair_type;


        typedef boost::unordered_set<std::string> name_set_type;

        name_set_type names_in;
        name_set_type names_out;

        for ( const_iterator p (trans.ports_begin())
            ; p != trans.ports_end()
            ; ++p
            )
          {
            if (p->second.is_input())
              {
                names_in.insert (p->second.name());
              }
            else
              {
                names_out.insert (p->second.name());
              }
          }

        for ( name_set_type::const_iterator n (names_in.begin())
            ; n != names_in.end()
            ; ++n
            )
          {
            if (names_out.find (*n) != names_out.end())
              {
                return fhg::util::Nothing<pair_type>();
              }
          }

        boost::unordered_set<pair_type> transs;

        for ( adj_place_const_it p (net.in_to_transition (tid))
            ; p.has_more()
            ; ++p
            )
          {
            if (net.in_to_place (*p).size() != 1)
              {
                return fhg::util::Nothing<pair_type>();
              }

            for ( adj_transition_const_it t (net.in_to_place (*p))
                ; t.has_more()
                ; ++t
                )
              {
                const petri_net::tid_t & tid_pred (*t);
                const transition_t & trans (net.get_transition (tid_pred));

                transs.insert (pair_type (trans, tid_pred));

                if (  (transs.size() > 1)
                   || (  boost::apply_visitor (content::visitor(), trans.data())
                      != content::expression
                      )
                   )
                  {
                    return fhg::util::Nothing<pair_type>();
                  }
              }
          }

        if (transs.size() != 1)
          {
            return fhg::util::Nothing<pair_type>();
          }

        return *transs.begin();
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void resolve_ports
      ( transition_t<P, E, T> & trans
      , const petri_net::tid_t & tid_trans
      , const transition_t<P, E, T> & pred
      , const petri_net::net<P, transition_t<P, E, T>, E, T> & net
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef typename transition_t::port_id_t port_id_t;
        typedef typename transition_t::port_t port_t;
        typedef petri_net::net<P, transition_t, E, T> pnet_t;
        typedef petri_net::adj_place_const_it adj_place_const_it;

        expression_t & expression (boost::get<expression_t &> (trans.data()));

        for ( adj_place_const_it p (net.in_to_transition (tid_trans))
            ; p.has_more()
            ; ++p
            )
          {
            const port_t & pred_out
              (pred.get_port (pred.output_port_by_pid (*p).first));

            port_t & trans_in
              (trans.get_port (trans.input_port_by_pid (*p).first));

            expression.rename (trans_in.name(), pred_out.name());

            trans_in.name() = pred_out.name();
          }
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void rename_ports
      ( transition_t<P, E, T> & trans
      , const transition_t<P, E, T> & other
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef typename transition_t::port_iterator port_iterator;
        typedef typename transition_t::const_iterator const_iterator;
        typedef typename transition_t::port_t port_t;

        boost::unordered_set<std::string> other_names;

        for ( const_iterator p (other.ports_begin())
            ; p != other.ports_end()
            ; ++p
            )
          {
            other_names.insert (p->second.name());
          }

        const std::string prefix (rewrite::mk_prefix (trans.name()));

        expression_t & expression (boost::get<expression_t &> (trans.data()));

        for (port_iterator p (trans.ports_begin()); p != trans.ports_end(); ++p)
          {
            port_t & port (p->second);

            if (other_names.find (port.name()) != other_names.end())
              {
                expression.rename (port.name(), prefix + port.name());

                port.name() = prefix + port.name();
              }
          }
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void take_ports
      ( const transition_t<P, E, T> & trans
      , const petri_net::tid_t tid_trans
      , transition_t<P, E, T> & pred
      , const petri_net::tid_t tid_pred
      , petri_net::net<P, transition_t<P, E, T>, E, T> & net
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef typename transition_t::const_iterator const_iterator;
        typedef typename transition_t::port_t port_t;
        typedef typename transition_t::port_id_t port_id_t;
        typedef petri_net::pid_t pid_t;
        typedef petri_net::eid_t eid_t;
        typedef petri_net::connection_t connection_t;

        for ( const_iterator p (trans.ports_begin())
            ; p != trans.ports_end()
            ; ++p
            )
          {
            if (p->second.is_output())
              {
                pred.UNSAFE_add_port (p->second);

                const pid_t pid (trans.inner_to_outer (p->first));

                const eid_t eid (net.get_eid_out (tid_trans, pid));
                const E edge (net.get_edge (eid));
                connection_t connection (net.get_edge_info (eid));

                net.delete_edge (eid);

                connection.tid = tid_pred;

                net.add_edge (edge, connection);

                pred.add_connections ()
                  (p->second.name(), pid, p->second.property())
                  ;
              }
          }
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void clear_ports
      ( transition_t<P, E, T> & trans
      , const petri_net::tid_t tid_trans
      , const transition_t<P, E, T> & trans_parent
      , petri_net::net<P, transition_t<P, E, T>, E, T> & net
      )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef typename transition_t::const_iterator const_iterator;
        typedef typename transition_t::port_t port_t;
        typedef typename transition_t::port_id_t port_id_t;
        typedef petri_net::pid_t pid_t;
        typedef petri_net::eid_t eid_t;

        typedef std::pair<port_id_t, pid_t> pair_type;
        std::stack<pair_type> to_erase;

        for ( const_iterator p (trans.ports_begin())
            ; p != trans.ports_end()
            ; ++p
            )
          {
            if (p->second.is_output())
              {
                const pid_t pid (trans.inner_to_outer (p->first));

                if (  net.out_of_place (pid).empty()
                   && !is_associated (trans_parent, pid)
                   )
                  {
                    to_erase.push (pair_type (p->first, pid));
                  }
              }
          }

        while (!to_erase.empty())
          {
            const pair_type & p (to_erase.top());
            const port_id_t & port_id (p.first);
            const pid_t & pid (p.second);

            net.delete_place (pid);
            trans.erase_port (port_id);

            to_erase.pop();
          }
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline bool run
      ( transition_t<P, E, T> & trans_parent
      , petri_net::net<P, transition_t<P,E,T>, E, T> & net
      )
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
            const tid_t & tid_trans (stack.top());
            transition_t trans (net.get_transition (tid_trans));

            if (  (  boost::apply_visitor (content::visitor (), trans.data())
                  == content::expression
                  )
               && trans.condition().is_const_true()
               )
              {
                const fhg::util::maybe<std::pair< const transition_t &
                                                , const tid_t &
                                                >
                                      >
                  maybe_pred (expression_predecessor (trans, tid_trans, net));

                if (maybe_pred.isJust())
                  {
                    transition_t pred ((*maybe_pred).first);
                    tid_t tid_pred ((*maybe_pred).second);

                    rename_ports<P, E, T> (trans, pred);

                    resolve_ports<P, E, T>(trans, tid_trans, pred, net);

                    expression_t & exp_trans
                      (boost::get<expression_t &> (trans.data()));

                    expression_t & exp_pred
                      (boost::get<expression_t &> (pred.data()));

                    exp_pred.add (exp_trans);

                    take_ports<P, E, T> (trans, tid_trans, pred, tid_pred, net);

                    net.delete_transition (tid_trans);

                    clear_ports<P, E, T> (pred, tid_pred, trans_parent, net);

                    net.modify_transition (tid_pred, pred);

                    modified = true;
                  }
              }

            stack.pop();
          }

        return modified;
      }
    }}
  }
}

#endif
