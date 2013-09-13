// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_PLACES_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_MERGE_PLACES_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>
#include <we/type/place.hpp>

#include <we/type/net.hpp>

#include <stack>

#include <rewrite/validprefix.hpp>

#include <boost/range/adaptor/map.hpp>

namespace we { namespace type {
    namespace optimize
    {
      inline void merge_places
      ( petri_net::net & net
      , const petri_net::place_id_type & pid_A
      , const bool & is_read
      , const petri_net::place_id_type & pid_B
      )
      {
        typedef petri_net::net pnet_t;
        typedef petri_net::connection_t connection_t;

        typedef std::stack<petri_net::transition_id_type> stack_t;

        stack_t stack;

        // rewire pid_B -> trans to pid_A -> trans
        BOOST_FOREACH ( const petri_net::transition_id_type& trans_out_B
                      , net.out_of_place (pid_B) | boost::adaptors::map_keys
                      )
          {
            stack.push (trans_out_B);
          }

        while (!stack.empty())
          {
            const petri_net::transition_id_type& tid_trans_out_B (stack.top());

            connection_t connection
              (net.get_connection_in (tid_trans_out_B, pid_B));

            net.delete_edge_in (tid_trans_out_B, pid_B);

            connection.pid = pid_A;

            if (is_read)
              {
                connection.type = petri_net::edge::PT_READ;
              }

            net.add_connection (connection);

            transition_t trans_out_B (net.get_transition (tid_trans_out_B));

            transition_t::port_id_with_prop_t port_id_with_prop
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
        BOOST_FOREACH ( const petri_net::transition_id_type& transition_id
                      , net.in_to_place (pid_B) | boost::adaptors::map_keys
                      )
          {
            stack.push (transition_id);
          }

        while (!stack.empty())
          {
            const petri_net::transition_id_type& tid_trans_in_B (stack.top());

            connection_t connection
              (net.get_connection_out (tid_trans_in_B, pid_B));

            net.delete_edge_out (tid_trans_in_B, pid_B);

            connection.pid = pid_A;

            net.add_connection (connection);

            transition_t trans_in_B (net.get_transition (tid_trans_in_B));

            transition_t::port_id_with_prop_t port_id_with_prop
              (trans_in_B.output_port_by_pid (pid_B));

            trans_in_B.re_connect_inner_to_outer
              ( port_id_with_prop.first
              , pid_A
              , port_id_with_prop.second
              );

            net.modify_transition (tid_trans_in_B, trans_in_B);

            stack.pop();
          }

        // take the better name
        const std::string name_A (net.get_place (pid_A).name());
        const std::string name_B (net.get_place (pid_B).name());

        std::list<pnet::type::value::value_type> tokens (net.get_token (pid_B));

        net.delete_place (pid_B);

        BOOST_FOREACH (const pnet::type::value::value_type& token, tokens)
        {
          net.put_token (pid_A, token);
        }

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
              , place_old.signature()
              , place_old.property()
              );

            net.modify_place (pid_A, place_new);
          }
      }
    }
  }
}

#endif
