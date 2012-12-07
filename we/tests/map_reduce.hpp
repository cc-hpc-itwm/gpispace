#ifndef WE_TESTS_MAP_REDUCE_HPP
#define WE_TESTS_MAP_REDUCE_HPP 1

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

namespace we
{
  namespace tests
  {
    using petri_net::connection_t;
    using petri_net::edge::PT;
    using petri_net::edge::PT_READ;
    using petri_net::edge::TP;

    template <typename Activity>
    struct map_reduce
    {
      typedef typename Activity::transition_type transition_type;

      typedef unsigned int edge_type;

      typedef petri_net::net net_type;
      typedef we::type::module_call_t mod_type;
      typedef we::type::expression_t expr_type;
      typedef std::string cond_type;

      typedef petri_net::pid_t pid_t;
      typedef petri_net::tid_t tid_t;

      typedef typename transition_type::port_id_t port_id_t;

      static transition_type generate ()
      {
        transition_type map_reduce
          ( "map"
          , mod_type ("map_red", "map")
          , cond_type("true")
          , transition_type::external
          );

        map_reduce.add_port ("in", literal::STRING(), we::type::PORT_IN);
        map_reduce.add_port ("out", literal::STRING(), we::type::PORT_OUT);

        return map_reduce;
      }
    };
  }
}

#endif
