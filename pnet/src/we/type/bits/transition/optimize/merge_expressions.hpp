// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_EXPRESSIONS_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_EXPRESSIONS_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>
#include <we/type/port.hpp>

#include <we/type/net.hpp>

#include <rewrite/validprefix.hpp>

#include <stack>

#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

namespace we { namespace type {
    namespace optimize { namespace merge_expressions
    {
      // ******************************************************************* //

      inline boost::optional<const petri_net::place_id_type&>
      input_pid_by_port_id ( const transition_t& trans
                           , const petri_net::port_id_type& port_id
                           )
      {
        BOOST_FOREACH ( transition_t::outer_to_inner_t::value_type const& p
                      , trans.outer_to_inner()
                      )
        {
          if (p.second.first == port_id)
          {
            return p.first;
          }
        }

        return boost::none;
      }

      inline boost::optional<const we::type::port_t>
      minput_port_by_pid ( const transition_t & trans
                         , const petri_net::place_id_type & pid
                         )
      {
        try
          {
            return trans.get_port (input_port_by_pid (trans, pid).first);
          }
        catch (const we::type::exception::not_connected<petri_net::place_id_type> &)
          {
            return boost::none;
          }
      }

      // ******************************************************************* //

      struct trans_info
      {
        typedef boost::unordered_set<petri_net::place_id_type> pid_set_type;

        const transition_t pred;
        const petri_net::transition_id_type tid_pred;
        const pid_set_type pid_read;

        trans_info ( const transition_t & _pred
                   , const petri_net::transition_id_type & _tid_pred
                   , const pid_set_type & _pid_read
                   )
          : pred (_pred), tid_pred (_tid_pred), pid_read (_pid_read)
        {}
      };

      inline boost::optional<trans_info>
      expression_predecessor
      ( const transition_t & trans
      , const petri_net::transition_id_type & tid
      , const petri_net::net & net
      )
      {
        typedef petri_net::net pnet_t;
        typedef petri_net::connection_t connection_t;
        typedef trans_info::pid_set_type pid_set_type;

        typedef std::pair<const transition_t, const petri_net::transition_id_type> pair_type;
        typedef boost::unordered_set<pair_type> set_of_pair_type;

        typedef std::pair< const petri_net::transition_id_type
                         , const petri_net::place_id_type
                         > tid_pid_type;
        typedef boost::unordered_set<tid_pid_type> set_of_tid_pid_type;

        typedef boost::unordered_set<std::string> name_set_type;

        // no chance when input and output ports have the same name
        name_set_type names_in;
        name_set_type names_out;

        BOOST_FOREACH ( we::type::port_t const& port
                      , trans.ports() | boost::adaptors::map_values
                      )
        {
          if (port.is_input())
          {
            names_in.insert (port.name());
          }
          else
          {
            names_out.insert (port.name());
          }
        }

        for ( name_set_type::const_iterator n (names_in.begin())
            ; n != names_in.end()
            ; ++n
            )
          {
            if (names_out.find (*n) != names_out.end())
              {
                return boost::none;
              }
          }

        // collect outgoing pids
        pid_set_type pid_out;

        BOOST_FOREACH ( const petri_net::place_id_type& place_id
                      , net.out_of_transition (tid) | boost::adaptors::map_keys
                      )
        {
          pid_out.insert (place_id);
        }

        // collect predecessors, separate read connections
        set_of_pair_type preds;
        set_of_tid_pid_type preds_read;
        pid_set_type pid_read;
        std::size_t max_successors_of_pred = 0;

        typedef std::pair<petri_net::place_id_type, connection_t> pc_type;

        BOOST_FOREACH (const pc_type& pc, net.in_to_transition (tid))
        {
          const connection_t& connection (pc.second);
          const petri_net::place_id_type& place_id (pc.first);

          if (petri_net::edge::is_pt_read (connection.type))
          {
            if (net.in_to_place (place_id).empty())
            {
              pid_read.insert (place_id);
            }
            else
            {
              BOOST_FOREACH ( const petri_net::transition_id_type& transition_id
                            , net.in_to_place (place_id)
                            | boost::adaptors::map_keys
                            )
              {
                preds_read.insert (tid_pid_type (transition_id, place_id));

                BOOST_FOREACH ( const petri_net::place_id_type& out_place_id
                              , net.out_of_transition (transition_id)
                              | boost::adaptors::map_keys
                              )
                {
                  if (pid_out.find (out_place_id) != pid_out.end())
                  {
                    return boost::none;
                  }
                }
              }
            }
          }
          else if (net.in_to_place (place_id).empty())
          {
            // WORK HERE: possible optimization: make the place an
            // input place of the only one predecessor
            // BEWARE: check the conditions!
            return boost::none;
          }
          else
          {
            BOOST_FOREACH ( const petri_net::transition_id_type& transition_id
                          , net.in_to_place (place_id)
                          | boost::adaptors::map_keys
                          )
            {
              const petri_net::transition_id_type & tid_pred (transition_id);
              const transition_t & trans_pred (net.get_transition (tid_pred));

              if (not content::is_expression (trans_pred))
              {
                return boost::none;
              }

              BOOST_FOREACH ( const petri_net::place_id_type& out_place_id
                            , net.out_of_transition (transition_id)
                            | boost::adaptors::map_keys
                            )
              {
                if (pid_out.find (out_place_id) != pid_out.end())
                {
                  return boost::none;
                }

                max_successors_of_pred =
                  std::max ( max_successors_of_pred
                           , net.out_of_place (out_place_id).size()
                           );
              }

              preds.insert (pair_type (trans_pred, tid_pred));
            }
          }
        }

        if (  (preds.size() != 1)
           || (!preds_read.empty() && max_successors_of_pred > 1)
           )
          {
            return boost::none;
          }

        const pair_type p (*preds.begin());

        for ( set_of_tid_pid_type::const_iterator tr (preds_read.begin())
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

      inline void resolve_ports
      ( transition_t & trans
      , const petri_net::transition_id_type & tid_trans
      , const transition_t & pred
      , const petri_net::net & net
      , const trans_info::pid_set_type & pid_read
      )
      {
        typedef we::type::port_t port_t;
        typedef petri_net::net pnet_t;

        expression_t & expression (boost::get<expression_t &> (trans.data()));

        BOOST_FOREACH ( const petri_net::place_id_type& place_id
                      , net.in_to_transition (tid_trans)
                      | boost::adaptors::map_keys
                      )
          {
            if (pid_read.find (place_id) == pid_read.end())
              {
                const port_t & pred_out
                  (pred.get_port (output_port_by_pid (pred, place_id).first));

                port_t & trans_in
                  (trans.get_port (input_port_by_pid (trans, place_id).first));

                expression.rename (trans_in.name(), pred_out.name());

                trans_in.name() = pred_out.name();
              }
            else
              {
                const boost::optional<const port_t>
                  maybe_pred_in (minput_port_by_pid (pred, place_id));

                if (maybe_pred_in)
                  {
                    const port_t & pred_in (*maybe_pred_in);

                    port_t & trans_in
                      (trans.get_port (input_port_by_pid (trans, place_id).first));

                    expression.rename (trans_in.name(), pred_in.name());

                    trans_in.name() = pred_in.name();
                  }
              }
          }
      }

      // ******************************************************************* //

      inline void rename_ports
      ( transition_t & trans
      , const transition_t & other
      )
      {
        typedef transition_t transition_t;
        typedef we::type::port_t port_t;

        boost::unordered_set<std::string> other_names;

        BOOST_FOREACH ( we::type::port_t const& port
                      , other.ports() | boost::adaptors::map_values
                      )
        {
          other_names.insert (port.name());
        }

        const std::string prefix (rewrite::mk_prefix (trans.name()));

        expression_t & expression (boost::get<expression_t &> (trans.data()));

        BOOST_FOREACH ( we::type::port_t& port
                      , trans.ports() | boost::adaptors::map_values
                      )
        {
          if (other_names.find (port.name()) != other_names.end())
          {
            expression.rename (port.name(), prefix + port.name());

            port.name() = prefix + port.name();
          }
        }
      }

      // ******************************************************************* //

      inline void take_ports
      ( const transition_t & trans
      , const petri_net::transition_id_type tid_trans
      , transition_t & pred
      , const petri_net::transition_id_type tid_pred
      , petri_net::net & net
      , const trans_info::pid_set_type pid_read
      )
      {
        typedef petri_net::connection_t connection_t;

        BOOST_FOREACH
          ( we::type::transition_t::port_map_t::value_type const& p
          , trans.ports()
          )
          {
            if (p.second.is_output())
              {
                pred.add_port (p.second);

                const petri_net::place_id_type pid
                  (trans.inner_to_outer().at (p.first).first);

                connection_t connection (net.get_connection_out (tid_trans, pid));

                net.delete_edge_out (tid_trans, pid);

                connection.tid = tid_pred;

                net.add_connection (connection);

                pred.add_connection (p.second.name(), pid, p.second.property());
              }
            else
            {
              const boost::optional<const petri_net::place_id_type&> pid
                (input_pid_by_port_id (trans, p.first));

              if (pid && pid_read.find (*pid) != pid_read.end())
              {
                if (not minput_port_by_pid (pred, *pid))
                {
                  pred.add_port (p.second);

                  connection_t const connection
                    (net.get_connection_in (tid_trans, *pid));

                  net.delete_edge_in (tid_trans, *pid);

                  net.add_connection (connection_t ( connection.type
                                                   , tid_pred
                                                   , connection.pid
                                                   )
                                     );

                  pred.add_connection
                    (*pid, p.second.name(), p.second.property())
                    ;
                }
              }
            }
          }
      }

      // ******************************************************************* //

      inline void clear_ports
      ( transition_t & trans
      , const petri_net::transition_id_type /* tid_trans */
      , const transition_t & trans_parent
      , petri_net::net & net
      )
      {
        typedef std::pair<petri_net::port_id_type, petri_net::place_id_type> pair_type;
        std::stack<pair_type> to_erase;

        BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                      , trans.ports()
                      )
        {
          if (p.second.is_output())
          {
            const petri_net::place_id_type pid
              (trans.inner_to_outer().at (p.first).first);

            namespace prop = we::type::property::traverse;

            //! \todo eliminate the hack that stores the real
            //! place in the properties
            prop::stack_type stack
              (prop::dfs (net.get_place(pid).property(), "real"));

            if (  net.out_of_place (pid).empty()
               && stack.empty()
               && !get_port_by_associated_pid (trans_parent, pid)
               )
            {
              to_erase.push (pair_type (p.first, pid));
            }
          }
        }

        while (!to_erase.empty())
          {
            const pair_type & p (to_erase.top());
            const petri_net::port_id_type& port_id (p.first);
            const petri_net::place_id_type & pid (p.second);

            net.delete_place (pid);
            trans.erase_port (port_id);

            to_erase.pop();
          }
      }

      // ******************************************************************* //

      inline bool run_once
      ( transition_t & trans_parent
      , petri_net::net & net
      )
      {
        typedef petri_net::net pnet_t;

        bool modified (false);

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
            const petri_net::transition_id_type & tid_trans (stack.top());
            transition_t trans (net.get_transition (tid_trans));

            if (  content::is_expression (trans)
               && trans.condition().is_const_true()
               )
              {
                typedef trans_info::pid_set_type pid_set_type;

                const boost::optional<trans_info>
                  maybe_pred (expression_predecessor (trans, tid_trans, net));

                if (maybe_pred)
                  {
                    transition_t pred ((*maybe_pred).pred);
                    petri_net::transition_id_type tid_pred ((*maybe_pred).tid_pred);

                    pid_set_type pid_read ((*maybe_pred).pid_read);

                    rename_ports (trans, pred);

                    resolve_ports (trans, tid_trans, pred, net, pid_read);

                    expression_t & exp_trans
                      (boost::get<expression_t &> (trans.data()));

                    expression_t & exp_pred
                      (boost::get<expression_t &> (pred.data()));

                    exp_pred.add (exp_trans);

                    take_ports (trans, tid_trans, pred, tid_pred, net, pid_read);

                    net.delete_transition (tid_trans);

                    clear_ports (pred, tid_pred, trans_parent, net);

                    net.modify_transition (tid_pred, pred);

                    modified = true;
                  }
              }

            stack.pop();
          }

        return modified;
      }

      inline bool run
      ( transition_t & trans_parent
      , petri_net::net & net
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
