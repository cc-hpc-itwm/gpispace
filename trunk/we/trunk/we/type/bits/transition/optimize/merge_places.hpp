// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_PLACES_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_PLACES_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>

#include <stack>

#include <rewrite/validprefix.hpp>

#include <fhg/util/maybe.hpp>

namespace we { namespace type {
    namespace optimize
    {
      template<typename P, typename E, typename T>
      inline void merge_places
      ( petri_net::net<P, transition_t<P,E,T>, E, T> & net
      , const petri_net::pid_t & pid_A
      , const bool & is_read
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

            if (is_read)
              {
                connection.type = petri_net::pt_read();
              }

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

        // capacities
        typedef petri_net::capacity_t capacity_t;

        fhg::util::maybe<capacity_t> cap_A (net.get_maybe_capacity (pid_A));
        fhg::util::maybe<capacity_t> cap_B (net.get_maybe_capacity (pid_B));

        if (cap_A.isJust())
          {
            if (cap_B.isJust())
              {
                net.set_capacity (pid_A, std::min (*cap_A, *cap_B));
              }
          }
        else
          {
            if (cap_B.isJust())
              {
                net.set_capacity (pid_A, *cap_B);
              }
          }

        // take the better name
        const std::string name_A (place::name (net, pid_A));
        const std::string name_B (place::name (net, pid_B));

        net.delete_place (pid_B);

        const bool okay_A (!rewrite::has_magic_prefix (name_A));
        const bool okay_B (!rewrite::has_magic_prefix (name_B));

        if (  (  (  ( okay_A &&  okay_B)
                 || (!okay_A && !okay_B)
                 )
              && (name_A.length() > name_B.length())
              )
           || (!okay_A && okay_B)
           )
          {
            const place::type place_old (net.get_place (pid_A));
            const place::type place_new
              ( name_B
              , place_old.get_signature()
              , place_old.get_property()
              );

            net.modify_place (pid_A, place_new);
          }
      }
    }
  }
}

#endif
