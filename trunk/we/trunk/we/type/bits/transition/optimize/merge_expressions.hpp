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
      inline fhg::util::maybe<const typename transition_t<P, E, T>::port_t>
      input_port_by_pid ( const transition_t<P, E, T> & trans
                        , const petri_net::pid_t & pid
                        )
      {
        typedef transition_t<P, E, T> transition_t;
        typedef typename transition_t::port_t port_t;

        try
          {
            return fhg::util::Just<const port_t>
              (trans.get_port (trans.input_port_by_pid (pid).first));
          }
        catch (const we::type::exception::not_connected<petri_net::pid_t> &)
          {
            return fhg::util::Nothing<const port_t>();
          }
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      struct trans_info
      {
        typedef boost::unordered_set<petri_net::pid_t> pid_set_type;

        const transition_t<P, E, T> pred;
        const petri_net::tid_t tid_pred;
        const pid_set_type pid_read;

        trans_info ( const transition_t<P, E, T> & _pred
                   , const petri_net::tid_t & _tid_pred
                   , const pid_set_type & _pid_read
                   )
          : pred (_pred), tid_pred (_tid_pred), pid_read (_pid_read)
        {}
      };

      template<typename P, typename E, typename T>
      inline fhg::util::maybe<trans_info<P, E, T> >
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
        typedef petri_net::connection_t connection_t;
        typedef typename transition_t::const_iterator const_iterator;
        typedef trans_info<P, E, T> trans_info;
        typedef typename trans_info::pid_set_type pid_set_type;

        typedef std::pair<const transition_t, const tid_t> pair_type;
        typedef boost::unordered_set<pair_type> set_of_pair_type;

        typedef std::pair<const tid_t, const pid_t> tid_pid_type;
        typedef boost::unordered_set<tid_pid_type> set_of_tid_pid_type;

        typedef boost::unordered_set<std::string> name_set_type;

        // no chance when input and output ports have the same name
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
                return fhg::util::Nothing<trans_info>();
              }
          }

        // collect outgoing pids
        pid_set_type pid_out;

        for ( adj_place_const_it p (net.out_of_transition (tid))
            ; p.has_more()
            ; ++p
            )
          {
            pid_out.insert (*p);
          }

        // collect predecessors, separate read connections
        set_of_pair_type preds;
        set_of_tid_pid_type preds_read;
        pid_set_type pid_read;

        for ( adj_place_const_it p (net.in_to_transition (tid))
            ; p.has_more()
            ; ++p
            )
          {
            const connection_t & connection (net.get_edge_info (p()));

            if (petri_net::is_pt_read (connection.type))
              {
                if (net.in_to_place (*p).empty())
                  {
                    pid_read.insert (*p);
                  }
                else
                  {
                    for ( adj_transition_const_it t (net.in_to_place (*p))
                        ; t.has_more()
                        ; ++t
                        )
                      {
                        preds_read.insert (tid_pid_type (*t, *p));

                        for ( adj_place_const_it tp (net.out_of_transition (*t))
                            ; tp.has_more()
                            ; ++tp
                            )
                          {
                            if (pid_out.find (*tp) != pid_out.end())
                              {
                                return fhg::util::Nothing<trans_info>();
                              }
                          }
                     }
                  }
              }
            else
              {
                for ( adj_transition_const_it t (net.in_to_place (*p))
                    ; t.has_more()
                    ; ++t
                    )
                  {
                    const petri_net::tid_t & tid_pred (*t);
                    const transition_t & trans (net.get_transition (tid_pred));

                    if (  boost::apply_visitor (content::visitor(), trans.data())
                       != content::expression
                       )
                      {
                        return fhg::util::Nothing<trans_info>();
                      }

                    for ( adj_place_const_it tp (net.out_of_transition (*t))
                        ; tp.has_more()
                        ; ++tp
                        )
                      {
                        if (pid_out.find (*tp) != pid_out.end())
                          {
                            return fhg::util::Nothing<trans_info>();
                          }
                      }

                    preds.insert (pair_type (trans, tid_pred));
                  }
              }
          }

        if (preds.size() != 1)
          {
            return fhg::util::Nothing<trans_info>();
          }

        const pair_type p (*preds.begin());

        for ( typename set_of_tid_pid_type::const_iterator tr (preds_read.begin())
            ; tr != preds_read.end()
            ; ++tr
            )
          {
            if (tr->first != p.second)
              {
                pid_read.insert (tr->second);
              }
          }

        return trans_info (p.first, p.second, pid_read);
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void resolve_ports
      ( transition_t<P, E, T> & trans
      , const petri_net::tid_t & tid_trans
      , const transition_t<P, E, T> & pred
      , const petri_net::net<P, transition_t<P, E, T>, E, T> & net
      , const typename trans_info<P, E, T>::pid_set_type & pid_read
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
            if (pid_read.find (*p) == pid_read.end())
              {
                const port_t & pred_out
                  (pred.get_port (pred.output_port_by_pid (*p).first));

                port_t & trans_in
                  (trans.get_port (trans.input_port_by_pid (*p).first));

                expression.rename (trans_in.name(), pred_out.name());

                trans_in.name() = pred_out.name();
              }
            else
              {
                const fhg::util::maybe<const port_t>
                  maybe_pred_in (input_port_by_pid (pred, *p));

                if (maybe_pred_in.isJust())
                  {
                    const port_t & pred_in (*maybe_pred_in);

                    port_t & trans_in
                      (trans.get_port (trans.input_port_by_pid (*p).first));

                    expression.rename (trans_in.name(), pred_in.name());

                    trans_in.name() = pred_in.name();
                  }
              }
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
      , const typename trans_info<P, E, T>::pid_set_type pid_read
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
            else
              {
                try
                  {
                    const pid_t pid (trans.input_pid_by_port_id (p->first));

                    if (pid_read.find (pid) != pid_read.end())
                      {
                        if (input_port_by_pid (pred, pid).isNothing())
                          {
                            pred.UNSAFE_add_port (p->second);

                            const eid_t eid (net.get_eid_in (tid_trans, pid));
                            const E edge (net.get_edge (eid));
                            connection_t connection (net.get_edge_info (eid));

                            net.delete_edge (eid);

                            connection.tid = tid_pred;

                            net.add_edge (edge, connection);

                            pred.add_connections ()
                              (pid, p->second.name(), p->second.property())
                              ;
                          }
                      }
                  }
                catch (const we::type::exception::not_connected<pid_t> &)
                  {
                    // do nothing, the port was not connected
                  }
              }
          }
      }

      // ******************************************************************* //

      template<typename P, typename E, typename T>
      inline void clear_ports
      ( transition_t<P, E, T> & trans
      , const petri_net::tid_t /* tid_trans */
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

                namespace prop = we::type::property::traverse;

                prop::stack_type stack
                  (prop::dfs (net.get_place(pid).get_property(), "real"));

                if (  net.out_of_place (pid).empty()
                   && stack.empty()
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
      inline bool run_once
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
                typedef trans_info<P, E, T> trans_info;
                typedef typename trans_info::pid_set_type pid_set_type;

                const fhg::util::maybe<trans_info>
                  maybe_pred (expression_predecessor (trans, tid_trans, net));

                if (maybe_pred.isJust())
                  {
                    transition_t pred ((*maybe_pred).pred);
                    tid_t tid_pred ((*maybe_pred).tid_pred);

                    pid_set_type pid_read ((*maybe_pred).pid_read);

                    rename_ports<P, E, T> (trans, pred);

                    resolve_ports<P, E, T>(trans, tid_trans, pred, net, pid_read);

                    expression_t & exp_trans
                      (boost::get<expression_t &> (trans.data()));

                    expression_t & exp_pred
                      (boost::get<expression_t &> (pred.data()));

                    exp_pred.add (exp_trans);

                    take_ports<P, E, T> (trans, tid_trans, pred, tid_pred, net, pid_read);

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

      template<typename P, typename E, typename T>
      inline bool run
      ( transition_t<P, E, T> & trans_parent
      , petri_net::net<P, transition_t<P,E,T>, E, T> & net
      )
      {
        bool modified (false);

        while (run_once (trans_parent, net))
          {
            modified = true;
          }

        return modified;
      }
    }}
  }
}

#endif
