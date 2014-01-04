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

namespace we
{
  namespace type
  {
    namespace optimize
    {
      namespace merge_expressions
      {
        // ***************************************************************** //

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
          minput_port_by_pid ( const transition_t& trans
                             , const petri_net::place_id_type& pid
                             )
        {
          boost::optional<transition_t::port_id_with_prop_t const&> pwp
            (input_port_by_pid (trans, pid));

          if (pwp)
          {
            return trans.get_port (pwp->first);
          }

          return boost::none;
        }

        // ***************************************************************** //

        struct trans_info
        {
          const transition_t pred;
          const petri_net::transition_id_type tid_pred;
          const boost::unordered_set<petri_net::place_id_type> pid_read;

          trans_info
            ( const transition_t& _pred
            , const petri_net::transition_id_type& _tid_pred
            , const boost::unordered_set<petri_net::place_id_type>& _pid_read
            )
            : pred (_pred), tid_pred (_tid_pred), pid_read (_pid_read)
          {}
        };

        inline boost::optional<trans_info>
          expression_predecessor
          ( const transition_t& trans
          , const petri_net::transition_id_type& tid
          , const petri_net::net& net
          )
        {
          typedef std::pair< const petri_net::transition_id_type
                           , const petri_net::place_id_type
                           > tid_pid_type;

          // no chance when input and output ports have the same name
          boost::unordered_set<std::string> names_in;
          boost::unordered_set<std::string> names_out;

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

          BOOST_FOREACH (std::string const& name_in, names_in)
          {
            if (names_out.find (name_in) != names_out.end())
            {
              return boost::none;
            }
          }

          // collect outgoing pids
          boost::unordered_set<petri_net::place_id_type> pid_out;

          BOOST_FOREACH
            ( const petri_net::place_id_type& place_id
            , net.out_of_transition (tid)
            )
          {
            pid_out.insert (place_id);
          }

          // collect predecessors, separate read connections
          boost::unordered_set<std::pair< const transition_t
                                        , const petri_net::transition_id_type
                                        >
                              > preds;
          boost::unordered_set<tid_pid_type> preds_read;
          boost::unordered_set<petri_net::place_id_type> pid_read;
          long max_successors_of_pred (0);

          typedef std::pair< petri_net::place_id_type
                           , petri_net::connection_t
                           > pc_type;

          BOOST_FOREACH (const pc_type& pc, net.in_to_transition (tid))
          {
            const petri_net::connection_t& connection (pc.second);
            const petri_net::place_id_type& place_id (pc.first);

            if (petri_net::edge::is_pt_read (connection.type()))
            {
              if (net.in_to_place (place_id).empty())
              {
                pid_read.insert (place_id);
              }
              else
              {
                BOOST_FOREACH
                  ( const petri_net::transition_id_type& transition_id
                  , net.in_to_place (place_id)
                  )
                {
                  preds_read.insert (std::make_pair (transition_id, place_id));

                  BOOST_FOREACH ( const petri_net::place_id_type& out_place_id
                                , net.out_of_transition (transition_id)
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
                            )
              {
                const petri_net::transition_id_type& tid_pred (transition_id);
                const transition_t& trans_pred (net.get_transition (tid_pred));

                if (not trans_pred.expression())
                {
                  return boost::none;
                }

                BOOST_FOREACH ( const petri_net::place_id_type& out_place_id
                              , net.out_of_transition (transition_id)
                              )
                {
                  if (pid_out.find (out_place_id) != pid_out.end())
                  {
                    return boost::none;
                  }

                  max_successors_of_pred =
                    std::max ( max_successors_of_pred
                             , boost::distance (net.out_of_place (out_place_id))
                             );
                }

                preds.insert (std::make_pair (trans_pred, tid_pred));
              }
            }
          }

          if (  (preds.size() != 1)
             || (!preds_read.empty() && max_successors_of_pred > 1)
             )
          {
            return boost::none;
          }

          const std::pair< const transition_t
                         , const petri_net::transition_id_type
                         > p (*preds.begin());

          BOOST_FOREACH (tid_pid_type const& tr, preds_read)
          {
            if (tr.first != p.second)
            {
              pid_read.insert (tr.second);
            }
          }

          return trans_info (p.first, p.second, pid_read);
        }

        // ***************************************************************** //

        inline void resolve_ports
          ( transition_t& trans
          , const petri_net::transition_id_type& tid_trans
          , const transition_t& pred
          , const petri_net::net& net
          , const boost::unordered_set<petri_net::place_id_type> pid_read
          )
        {
          expression_t& expression (boost::get<expression_t&> (trans.data()));

          BOOST_FOREACH ( const petri_net::place_id_type& place_id
                        , net.in_to_transition (tid_trans)
                        | boost::adaptors::map_keys
                        )
          {
            if (pid_read.find (place_id) == pid_read.end())
            {
              const port_t& pred_out
                (pred.get_port (output_port_by_pid (pred, place_id)->first));

              port_t& trans_in
                (trans.get_port (input_port_by_pid (trans, place_id)->first));

              expression.rename (trans_in.name(), pred_out.name());

              trans_in.name() = pred_out.name();
            }
            else
            {
              const boost::optional<const port_t>
                maybe_pred_in (minput_port_by_pid (pred, place_id));

              if (maybe_pred_in)
              {
                const port_t& pred_in (*maybe_pred_in);

                port_t& trans_in
                  (trans.get_port (input_port_by_pid (trans, place_id)->first));

                expression.rename (trans_in.name(), pred_in.name());

                trans_in.name() = pred_in.name();
              }
            }
          }
        }

        // ***************************************************************** //

        inline void rename_ports
          ( transition_t& trans
          , const transition_t& other
          )
        {
          boost::unordered_set<std::string> other_names;

          BOOST_FOREACH ( we::type::port_t const& port
                        , other.ports() | boost::adaptors::map_values
                        )
          {
            other_names.insert (port.name());
          }

          const std::string prefix (rewrite::mk_prefix (trans.name()));

          expression_t& expression (boost::get<expression_t&> (trans.data()));

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

        // ***************************************************************** //

        inline void take_ports
          ( const transition_t& trans
          , const petri_net::transition_id_type tid_trans
          , transition_t& pred
          , const petri_net::transition_id_type tid_pred
          , petri_net::net& net
          , const boost::unordered_set<petri_net::place_id_type> pid_read
          )
        {
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

              net.delete_edge_out (tid_trans, pid);

              net.add_connection (petri_net::edge::TP, tid_pred, pid);

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

                  petri_net::connection_t const connection
                    (net.get_connection_in (tid_trans, *pid));

                  net.delete_edge_in (tid_trans, *pid);

                  net.add_connection
                    (connection.type(), tid_pred, connection.place_id());

                  pred.add_connection
                    (*pid, p.second.name(), p.second.property());
                }
              }
            }
          }
        }

        // ***************************************************************** //

        inline void clear_ports
          ( transition_t& trans
          , const petri_net::transition_id_type /* tid_trans */
          , const transition_t& trans_parent
          , petri_net::net& net
          )
        {
          std::stack<std::pair< petri_net::port_id_type
                              , petri_net::place_id_type
                              >
                    > to_erase;

          BOOST_FOREACH
            ( we::type::transition_t::port_map_t::value_type const& p
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
                to_erase.push (std::make_pair (p.first, pid));
              }
            }
          }

          while (!to_erase.empty())
          {
            const petri_net::port_id_type& port_id (to_erase.top().first);
            const petri_net::place_id_type& pid (to_erase.top().second);

            net.delete_place (pid);
            trans.erase_port (port_id);

            to_erase.pop();
          }
        }

        // ***************************************************************** //

        inline bool run_once
          ( transition_t& trans_parent
          , petri_net::net& net
          )
        {
          bool modified (false);

          std::stack<petri_net::transition_id_type> stack;

          BOOST_FOREACH ( const petri_net::transition_id_type& t
                        , net.transitions() | boost::adaptors::map_keys
                        )
          {
            stack.push (t);
          }

          while (!stack.empty())
          {
            const petri_net::transition_id_type& tid_trans (stack.top());
            transition_t trans (net.get_transition (tid_trans));
            boost::optional<we::type::expression_t const&> const
              exp_trans (trans.expression());

            if (exp_trans && trans.condition().is_const_true())
            {
              const boost::optional<trans_info>
                maybe_pred (expression_predecessor (trans, tid_trans, net));

              if (maybe_pred)
              {
                transition_t pred ((*maybe_pred).pred);
                petri_net::transition_id_type tid_pred ((*maybe_pred).tid_pred);

                boost::unordered_set<petri_net::place_id_type>
                  pid_read ((*maybe_pred).pid_read);

                rename_ports (trans, pred);

                resolve_ports (trans, tid_trans, pred, net, pid_read);

                expression_t& exp_pred
                  (boost::get<expression_t&> (pred.data()));

                exp_pred.add (*exp_trans);

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

        inline bool run (transition_t& trans_parent, petri_net::net& net)
        {
          bool modified (false);

          while (run_once (trans_parent, net))
          {
            modified = true;
          }

          return modified;
        }
      }
    }
  }
}

#endif
